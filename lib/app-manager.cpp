/**
 * @file          /kiran-menu/lib/app-manager.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include "lib/app-manager.h"

#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <glib/gstdio.h>

#include <cinttypes>
#include <sstream>

#include "lib/base.h"
#include "lib/common.h"

namespace Kiran
{
#define SNAP_SECURITY_LABEL_PREFIX "snap."
#define USER_APP_DIR ".config/kiran-applets/applications/"

AppManager::AppManager(WindowManager *window_manager) : window_manager_(window_manager),
                                                        system_app_monitor(nullptr),
                                                        user_app_monitor(nullptr)
{
}

AppManager::~AppManager()
{
    if (system_app_monitor)
        g_object_unref(system_app_monitor);

    if (user_app_monitor)
        g_object_unref(user_app_monitor);
}

AppManager *AppManager::instance_ = nullptr;
void AppManager::global_init(WindowManager *window_manager)
{
    instance_ = new AppManager(window_manager);
    instance_->init();
}

std::vector<std::shared_ptr<App>> AppManager::get_apps()
{
    std::vector<std::shared_ptr<App>> apps;

    for (auto iter = this->apps_.begin(); iter != this->apps_.end(); ++iter)
    {
        apps.push_back(iter->second);
    }
    return apps;
}

AppVec AppManager::get_apps_by_kind(AppKind kind)
{
    std::vector<std::shared_ptr<App>> apps;

    for (auto iter = this->apps_.begin(); iter != this->apps_.end(); ++iter)
    {
        if (((int)iter->second->get_kind() & (int)kind) != 0)
        {
            apps.push_back(iter->second);
        }
    }
    return apps;
}

AppVec AppManager::get_should_show_apps()
{
    std::vector<std::shared_ptr<App>> apps;

    for (auto iter = this->apps_.begin(); iter != this->apps_.end(); ++iter)
    {
        if (iter->second->should_show())
        {
            apps.push_back(iter->second);
        }
    }
    return apps;
}

AppVec AppManager::get_running_apps()
{
    AppVec apps;
    for (auto iter = this->wnck_apps_.begin(); iter != this->wnck_apps_.end(); ++iter)
    {
        if (!(iter->second.expired()))
        {
            apps.push_back(iter->second.lock());
        }
    }
    auto iter = std::unique(apps.begin(), apps.end(), [](decltype(*apps.begin()) a, decltype(*apps.begin()) b) {
        return a.get() == b.get();
    });
    if (iter != apps.end())
    {
        apps.erase(iter, apps.end());
    }
    return apps;
}

std::shared_ptr<App> AppManager::lookup_app(const std::string &desktop_id)
{
    RETURN_VAL_IF_TRUE(desktop_id.length() == 0, nullptr);

    auto iter = this->apps_.find(desktop_id);
    if (iter == this->apps_.end())
    {
        return nullptr;
    }
    else
    {
        return iter->second;
    }
}

std::shared_ptr<App> AppManager::lookup_app_with_window(std::shared_ptr<Window> window)
{
    RETURN_VAL_IF_FALSE(window != nullptr, nullptr);

    KLOG_DEBUG("lookup app for window: %s.", window->get_name().c_str());

    auto transient_for = window->get_transient();
    if (transient_for)
    {
        return lookup_app_with_window(transient_for);
    }

#define PRINT_AND_RETURN_MATCH_APP(app)                                                                         \
    if (app)                                                                                                    \
    {                                                                                                           \
        KLOG_DEBUG("match app: %s for window: %s.", app->get_desktop_id().c_str(), window->get_name().c_str()); \
        return app;                                                                                             \
    }

    std::shared_ptr<App> app;

    // 为了提高匹配效率，这里直接从缓存中查找，如果缓存中没有找到，说明是这个窗口是第一次调用该函数
    app = lookup_app_with_xid(window->get_window_group());
    PRINT_AND_RETURN_MATCH_APP(app);

    app = get_app_from_env(window);
    PRINT_AND_RETURN_MATCH_APP(app);

    app = get_app_from_sandboxed_app(window);
    PRINT_AND_RETURN_MATCH_APP(app);

    app = get_app_from_gapplication_id(window);
    PRINT_AND_RETURN_MATCH_APP(app);

    app = get_app_from_cmdline(window);
    PRINT_AND_RETURN_MATCH_APP(app);

    app = get_app_from_window_wmclass(window);
    PRINT_AND_RETURN_MATCH_APP(app);

    // 遍历所有app，检查是否有应用的属性跟窗口的wm_class属性匹配
    app = get_app_by_enumeration_apps(window);
    PRINT_AND_RETURN_MATCH_APP(app);

    // 遍历所有window，检查是否存在相同的wm_class属性的窗口已经匹配到app
    app = get_app_by_enumeration_windows(window);
    PRINT_AND_RETURN_MATCH_APP(app);

    app = get_app_from_window_group(window);
    PRINT_AND_RETURN_MATCH_APP(app);

    KLOG_WARNING("not found matching App for the window. name: %s xid: %" PRIu64 "\n",
                 window->get_name().c_str(),
                 window->get_xid());

    return nullptr;
}

std::shared_ptr<App> AppManager::lookup_app_with_xid(uint64_t xid)
{
    auto iter = this->wnck_apps_.find(xid);
    if (iter != this->wnck_apps_.end())
    {
        auto &app = iter->second;
        if (!app.expired())
        {
            return app.lock();
        }
    }
    return nullptr;
}

std::vector<std::string> AppManager::get_all_sorted_apps()
{
    std::vector<std::string> apps;

    for (auto iter = this->apps_.begin(); iter != this->apps_.end(); ++iter)
    {
        auto &app = iter->second;
        apps.push_back(app->get_desktop_id());
    }

    std::sort(apps.begin(), apps.end(), [this](std::string a, std::string b) -> bool {
        auto appa = this->lookup_app(a);
        auto appb = this->lookup_app(b);

        auto &appa_name = appa->get_name();
        auto &appb_name = appb->get_name();

        return appa_name < appb_name;
    });

    return apps;
}

void AppManager::init()
{
    KLOG_PROFILE("");

    load_desktop_apps();

    /* 监控用户应用目录 */
    GFile *user_app_dir = g_file_new_for_path(get_userapp_dir_path().c_str());
    user_app_monitor = g_file_monitor_directory(user_app_dir, G_FILE_MONITOR_NONE, NULL, NULL);
    g_return_if_fail(user_app_monitor != NULL);
    g_signal_connect_swapped(user_app_monitor, "changed", G_CALLBACK(AppManager::desktop_app_changed), this);
    g_object_unref(user_app_dir);

    /* 监控系统应用变化 */
    system_app_monitor = g_app_info_monitor_get();
    g_return_if_fail(system_app_monitor != NULL);
    g_signal_connect_swapped(system_app_monitor, "changed", G_CALLBACK(AppManager::desktop_app_changed), this);

    auto screen = wnck_screen_get_default();
    g_return_if_fail(screen != NULL);

    g_signal_connect(screen, "application-opened", G_CALLBACK(AppManager::app_opened), this);
    g_signal_connect(screen, "application-closed", G_CALLBACK(AppManager::app_closed), this);

    this->window_manager_->signal_window_opened().connect(sigc::mem_fun(this, &AppManager::window_opened));
    this->window_manager_->signal_window_closed().connect(sigc::mem_fun(this, &AppManager::window_closed));
}

std::shared_ptr<App> AppManager::get_app_from_sandboxed_app(std::shared_ptr<Window> window)
{
    std::shared_ptr<App> app;
    auto pid = window->get_pid();

    if (!pid)
    {
        return nullptr;
    }

    // check whether is flatpak app
    do
    {
        std::ostringstream oss;
        oss << "/proc/" << pid << "/root/.flatpak-info";

        auto file_path = oss.str();
        Glib::KeyFile key_file;
        bool load_result = true;
        try
        {
            load_result = key_file.load_from_file(file_path);
        }
        catch (const Glib::Exception &e)
        {
            load_result = false;
        }

        if (!load_result)
        {
            break;
        }

        auto desktop_id = key_file.get_string("Application", "name") + ".desktop";

        app = lookup_app(desktop_id.raw());
        RETURN_VAL_IF_TRUE(app, app);
    } while (0);

    // check whether is snap app
    do
    {
        std::ostringstream oss;
        oss << "/proc/" << pid << "/attr/current";

        auto file_path = oss.str();
        g_autofree char *security_label_contents = NULL;
        gsize i, security_label_contents_size = 0;

        auto file = Gio::File::create_for_path(file_path);
        if (!file)
        {
            break;
        }
        try
        {
            if (!file->load_contents(security_label_contents, security_label_contents_size))
            {
                break;
            }
        }
        catch (const Glib::Exception &e)
        {
            break;
        }

        if (!g_str_has_prefix(security_label_contents, SNAP_SECURITY_LABEL_PREFIX))
        {
            break;
        }

        char *contents_start;
        char *contents_end;

        /* We need to translate the security profile into the desktop-id.
        * The profile is in the form of 'snap.name-space.binary-name (current)'
        * while the desktop id will be name-space_binary-name.
        */
        security_label_contents_size -= sizeof(SNAP_SECURITY_LABEL_PREFIX) - 1;
        contents_start = security_label_contents + sizeof(SNAP_SECURITY_LABEL_PREFIX) - 1;
        contents_end = strchr(contents_start, ' ');

        if (contents_end)
        {
            security_label_contents_size = contents_end - contents_start;
        }

        for (i = 0; i < security_label_contents_size; ++i)
        {
            if (contents_start[i] == '.')
            {
                contents_start[i] = '_';
            }
        }

        auto desktop_id = std::string(contents_start, security_label_contents_size) + ".desktop";
        app = lookup_app(desktop_id);
        RETURN_VAL_IF_TRUE(app, app);
    } while (0);
    return nullptr;
}

std::shared_ptr<App> AppManager::get_app_from_gapplication_id(std::shared_ptr<Window> window)
{
    unsigned char *data = NULL;
    g_autofree char *name = NULL;
    Atom actual_type;
    int actual_format;
    unsigned long nitems, bytes_after;

    auto display = gdk_x11_get_default_xdisplay();
    auto gdk_display = gdk_x11_lookup_xdisplay(display);
    auto xid = window->get_xid();

    g_return_val_if_fail(display != NULL, nullptr);
    g_return_val_if_fail(gdk_display != NULL, nullptr);

    auto prop_atom = XInternAtom(display, "_GTK_APPLICATION_ID", true);

    gdk_x11_display_error_trap_push(gdk_display);

    auto result = XGetWindowProperty(display, xid, prop_atom, 0, G_MAXLONG, false,
                                     AnyPropertyType,
                                     &actual_type,
                                     &actual_format,
                                     &nitems,
                                     &bytes_after,
                                     &data);
    if (!gdk_x11_display_error_trap_pop(gdk_display) &&
        (result == Success) &&
        data != NULL)
    {
        name = strdup((char *)data);
        XFree(data);

        auto desktop_id = std::string(name) + std::string(".desktop");
        auto app = lookup_app(desktop_id);
        RETURN_VAL_IF_TRUE(app, app);
    }

    return nullptr;
}

std::shared_ptr<App> AppManager::get_app_from_cmdline(std::shared_ptr<Window> window)
{
    std::shared_ptr<App> app;
    auto pid = window->get_pid();

    if (!pid)
    {
        return nullptr;
    }

    std::ostringstream oss;
    oss << "/proc/" << pid << "/cmdline";

    auto file_path = oss.str();
    g_autofree char *file_contents = NULL;
    gsize file_size = 0;

    auto file = Gio::File::create_for_path(file_path);

    RETURN_VAL_IF_TRUE(!file, nullptr);

    try
    {
        if (!file->load_contents(file_contents, file_size))
        {
            return nullptr;
        }
    }
    catch (const Glib::Exception &e)
    {
        return nullptr;
    }

    auto cmdline = std::string(file_contents, file_size);

    auto split_cmdline = str_split(cmdline, " ");
    RETURN_VAL_IF_TRUE(split_cmdline.size() == 0, nullptr);

    auto basename = Glib::path_get_basename(split_cmdline[0]);

    return lookup_app_with_unrefine_name(basename);
}

std::shared_ptr<App> AppManager::get_app_from_window_wmclass(std::shared_ptr<Window> window)
{
    std::shared_ptr<App> app;

    auto wm_class_instance = window->get_class_instance_name();
    auto wm_class = window->get_class_group_name();

    // g_print("wm_class_instance: %s wm_class: %s\n", wm_class_instance.c_str(), wm_class.c_str());

    app = lookup_app_with_wmclass(wm_class_instance);
    RETURN_VAL_IF_TRUE(app, app);

    app = lookup_app_with_wmclass(wm_class);
    RETURN_VAL_IF_TRUE(app, app);

    app = lookup_app_with_unrefine_name(wm_class_instance);
    RETURN_VAL_IF_TRUE(app, app);

    app = lookup_app_with_unrefine_name(wm_class);
    RETURN_VAL_IF_TRUE(app, app);

    return nullptr;
}

std::shared_ptr<App> AppManager::lookup_app_with_wmclass(const std::string &wmclass)
{
    RETURN_VAL_IF_TRUE(wmclass.length() == 0, nullptr);

    auto iter = this->wmclass_apps_.find(wmclass);
    if (iter == this->wmclass_apps_.end() || iter->second.expired())
    {
        return nullptr;
    }
    else
    {
        return iter->second.lock();
    }
}

std::shared_ptr<App> AppManager::lookup_app_with_unrefine_name(const std::string &name)
{
    RETURN_VAL_IF_TRUE(name.length() == 0, nullptr);

    std::shared_ptr<App> app;

    std::string basename = name + std::string(".desktop");
    app = lookup_app_with_heuristic_name(basename);
    RETURN_VAL_IF_TRUE(app, app);

    std::string mainname = get_mainname(name);
    if (mainname != name)
    {
        basename = mainname + std::string(".desktop");
        app = lookup_app_with_heuristic_name(basename);
        RETURN_VAL_IF_TRUE(app, app);
    }

    std::string lower_wmclass = str_tolower(name);
    std::replace(lower_wmclass.begin(), lower_wmclass.end(), ' ', '-');
    if (lower_wmclass != name)
    {
        basename = lower_wmclass + std::string(".desktop");
        app = lookup_app_with_heuristic_name(basename);
        RETURN_VAL_IF_TRUE(app, app);
    }

    return nullptr;
}

std::shared_ptr<App> AppManager::lookup_app_with_heuristic_name(const std::string &name)
{
    std::shared_ptr<App> app;

    static const std::vector<std::string> vendor_prefixes{"gnome-", "fedora-", "mozilla-", "debian-"};

    app = lookup_app(name);
    RETURN_VAL_IF_TRUE(app, app);

    for (auto iter = vendor_prefixes.begin(); iter != vendor_prefixes.end(); ++iter)
    {
        std::string name_with_prefix = *iter + name;
        app = lookup_app(name_with_prefix);
        RETURN_VAL_IF_TRUE(app, app);
    }

    return nullptr;
}

static gchar **split_environ_str(gchar *str, int32_t len)
{
    GSList *string_list = NULL, *slist;
    gchar **str_array;
    guint n = 0;

    g_return_val_if_fail(str != NULL, NULL);
    g_return_val_if_fail(len != 0, NULL);

    int32_t last_pos = 0;

    for (int32_t i = 0; i <= len; ++i)
    {
        if ((str[i] == '\0' || i == len) && (i != last_pos))
        {
            string_list = g_slist_prepend(string_list, g_strndup(str + last_pos, i - last_pos));
            ++n;
            last_pos = i + 1;
        }
    }

    str_array = g_new(gchar *, n + 1);

    str_array[n--] = NULL;
    for (slist = string_list; slist; slist = slist->next)
        str_array[n--] = (gchar *)slist->data;

    g_slist_free(string_list);

    return str_array;
}

std::shared_ptr<App> AppManager::get_app_from_env(std::shared_ptr<Window> window)
{
    auto pid = window->get_pid();

    std::ostringstream oss;
    oss << "/proc/" << pid << "/environ";

    auto file_path = oss.str();
    g_autofree char *file_contents = NULL;
    gsize file_size = 0;

    auto file = Gio::File::create_for_path(file_path);
    if (!file)
    {
        return nullptr;
    }

    try
    {
        if (!file->load_contents(file_contents, file_size))
        {
            return nullptr;
        }
    }
    catch (const Glib::Exception &e)
    {
        return nullptr;
    }

    g_auto(GStrv) environ = split_environ_str(file_contents, file_size);

    auto desktop_file_name = g_environ_getenv(environ, "GIO_LAUNCHED_DESKTOP_FILE");
    RETURN_VAL_IF_TRUE(desktop_file_name == NULL, nullptr);

    auto desktop_id = Glib::path_get_basename(desktop_file_name);
    return lookup_app(desktop_id);
}

std::shared_ptr<App> AppManager::get_app_by_enumeration_apps(std::shared_ptr<Window> window)
{
    RETURN_VAL_IF_FALSE(window != NULL, nullptr);

    auto instance_name = window->get_class_instance_name();
    auto group_name = window->get_class_group_name();

    for (auto iter = this->apps_.begin(); iter != this->apps_.end(); ++iter)
    {
        auto &app = iter->second;
        auto &exec = app->get_exec();
        auto &locale_name = app->get_locale_name();
        auto exec_name = get_exec_name(exec);

        if (exec_name == instance_name || group_name == locale_name)
        {
            return app;
        }
    }

    return nullptr;
}

std::shared_ptr<App> AppManager::get_app_by_enumeration_windows(std::shared_ptr<Window> window)
{
    RETURN_VAL_IF_FALSE(window != NULL, nullptr);
    auto instance_name = window->get_class_instance_name();
    auto group_name = window->get_class_group_name();

    // 这里通过wnck application来遍历所有窗口
    // 因为如果是第一次调用，可能还处于"app-opened"处理阶段，此时"window-opened"信号可能还没有到达，WindowManager还没有管理任何窗口
    // 所以不能通过WindowManager::get_windows获取窗口列表
    for (auto iter : this->wnck_apps_)
    {
        auto wnck_application = wnck_application_get(iter.first);
        if (wnck_application)
        {
            auto wnck_windows = wnck_application_get_windows(wnck_application);
            for (auto l = wnck_windows; l != NULL; l = l->next)
            {
                auto wnck_window = (WnckWindow *)(l->data);
                auto temp_window = this->window_manager_->create_temp_window(wnck_window);
                if (temp_window->get_class_instance_name() == instance_name &&
                    temp_window->get_class_group_name() == group_name)
                {
                    auto app = lookup_app_with_xid(temp_window->get_window_group());
                    RETURN_VAL_IF_TRUE(app, app);
                }
            }
        }
    }
    return nullptr;
}

std::shared_ptr<App> AppManager::get_app_from_window_group(std::shared_ptr<Window> window)
{
    std::shared_ptr<App> app;

    auto windows = window->get_group_windows();
    for (auto iter = windows.begin(); iter != windows.end(); ++iter)
    {
        if (window.get() == (*iter).get())
        {
            continue;
        }

        app = get_app_from_env(*iter);
        RETURN_VAL_IF_TRUE(app, app);

        app = get_app_from_gapplication_id(window);
        RETURN_VAL_IF_TRUE(app, app);

        app = get_app_from_window_wmclass(*iter);
        RETURN_VAL_IF_TRUE(app, app);

        app = get_app_by_enumeration_apps(window);
        RETURN_VAL_IF_TRUE(app, app);
    }
    return nullptr;
}

std::string AppManager::gen_userapp_id(const std::string &userapp_dir,
                                       const std::string &desktop_id)
{
    /* 移除.desktop后缀 */
    int num = 1;
    std::string desktop_name;

    auto index = desktop_id.rfind(".desktop");
    if (index != std::string::npos)
    {
        desktop_name = desktop_id.substr(0, index);
    }
    else
        desktop_name = desktop_id;

    while (true)
    {
        char *new_id = nullptr, *desktop_path = nullptr;

        new_id = g_strdup_printf("userapp-%s-%d.desktop", desktop_name.c_str(), num);
        desktop_path = g_build_filename(userapp_dir.c_str(), new_id, NULL);
        if (!g_file_test(desktop_path, G_FILE_TEST_EXISTS))
        {
            desktop_name.assign(new_id);
            g_free(desktop_path);
            g_free(new_id);
            break;
        }

        g_free(desktop_path);
        g_free(new_id);
        num++;
    }

    return desktop_name;
}

std::string AppManager::get_userapp_dir_path()
{
    return Glib::ustring::compose("%1/" USER_APP_DIR, g_get_home_dir()).raw();
}

std::string AppManager::create_userapp_from_uri(const std::string &uri)
{
    Glib::RefPtr<Gio::File> source_file, dest_file;
    std::string new_id, dest_path, userapp_dir;
    Glib::RefPtr<Gio::DesktopAppInfo> source_app;

    try
    {
        source_file = Gio::File::create_for_uri(uri);
        if (!source_file)
            return "";

        /* 确保其为正常的desktop文件 */
        source_app = Gio::DesktopAppInfo::create_from_filename(source_file->get_path());
        if (!source_app)
            return "";
    }
    catch (const Gio::Error &e)
    {
        KLOG_ERROR("Failed to create user app for '%s': %s", uri.c_str(), e.what().c_str());
        return "";
    }

    /* 生成新Desktop文件名称 */
    userapp_dir = get_userapp_dir_path();
    new_id = gen_userapp_id(userapp_dir, source_app->get_id());
    dest_path = Glib::ustring::compose("%1/%2", userapp_dir, new_id);

    /* 确保用户应用目录存在 */
    if (g_mkdir_with_parents(userapp_dir.c_str(), 0755) < 0)
    {
        KLOG_ERROR("Error occured while trying to create user app directory '%s', error %d",
                   userapp_dir.c_str(),
                   errno);
        return "";
    }

    /* 拷贝desktop文件 */
    try
    {
        dest_file = Gio::File::create_for_path(dest_path);
        if (source_file->copy(dest_file))
            return new_id;
    }
    catch (Glib::Error &e)
    {
        KLOG_ERROR("Failed to copy file '%s': %s", source_file->get_path().c_str(), e.what().c_str());
    }

    return "";
}

bool AppManager::remove_app_from_disk(const std::string &desktop_id)
{
    auto app = lookup_app(desktop_id);
    if (!app || app->get_kind() != AppKind::USER_TASKBAR)
        return false;

    return g_unlink(app->get_file_name().c_str()) != -1;
}

void AppManager::load_desktop_apps()
{
    std::vector<std::shared_ptr<App>> installed_apps;
    std::vector<std::shared_ptr<App>> uninstalled_apps;

    // 缓存旧的app
    auto old_apps = this->apps_;

    this->clear_desktop_apps();
    this->wmclass_apps_.clear();

    // 加载用户自定义应用
    auto app_dir_path = get_userapp_dir_path();
    auto userapp_dir = g_file_new_for_path(app_dir_path.c_str());
    if (userapp_dir && g_file_query_file_type(userapp_dir, G_FILE_QUERY_INFO_NONE, NULL) == G_FILE_TYPE_DIRECTORY)
    {
        GFileEnumerator *enumerator = g_file_enumerate_children(userapp_dir, "*", G_FILE_QUERY_INFO_NONE, NULL, NULL);
        do
        {
            const char *child_name = nullptr;
            GFileInfo *info = g_file_enumerator_next_file(enumerator, NULL, NULL);
            if (!info)
                break;

            child_name = g_file_info_get_name(info);
            if (g_str_has_prefix(child_name, "userapp-") && g_str_has_suffix(child_name, ".desktop"))
            {
                auto file = g_file_enumerator_get_child(enumerator, info);
                KLOG_DEBUG("Found user desktop file '%s'", child_name);
                register_app(old_apps, g_file_get_path(file), AppKind::USER_TASKBAR);
                g_object_unref(file);
            }
            g_object_unref(info);
        } while (true);

        g_object_unref(enumerator);
        g_object_unref(userapp_dir);
    }

    // 加载系统应用
    auto registered_apps = Gio::AppInfo::get_all();
    for (auto iter = registered_apps.begin(); iter != registered_apps.end(); ++iter)
    {
        auto app = *iter;
        register_app(old_apps, app, AppKind::NORMAL);
    }

    // 更新WnckApplicatin和应用绑定
    this->update_wnckapps_binding();
    // 清理不再使用的fake app
    this->clear_nouse_fake_app();

    // 获取新安装的应用
    static bool first_flush = true;
    if (!first_flush)
    {
        for (auto &iter : this->apps_)
        {
            if (old_apps.find(iter.first) == old_apps.end())
            {
                installed_apps.push_back(iter.second);
            }
        }
    }
    else
    {
        first_flush = false;
    }

    // 获取卸载的应用
    for (auto iter = old_apps.begin(); iter != old_apps.end(); ++iter)
    {
        if (this->apps_.find(iter->first) == this->apps_.end())
        {
            uninstalled_apps.push_back(iter->second);
        }
    }

    if (installed_apps.size() > 0)
    {
        this->app_installed_.emit(installed_apps);
    }

    if (uninstalled_apps.size() > 0)
    {
        this->app_uninstalled_.emit(uninstalled_apps);
    }
}

void AppManager::clear_desktop_apps()
{
    for (auto iter = this->apps_.begin(); iter != this->apps_.end();)
    {
        AppKind kind = iter->second->get_kind();
        if (kind == AppKind::NORMAL || kind == AppKind::USER_TASKBAR)
        {
            this->apps_.erase(iter++);
            continue;
        }
        ++iter;
    }
}

void AppManager::register_app(std::map<std::string, std::shared_ptr<App>> &old_apps,
                              Glib::RefPtr<Gio::AppInfo> &app_,
                              AppKind kind)
{
    auto desktop_id = app_->get_id();
    std::shared_ptr<App> app;
    auto old_iter = old_apps.find(desktop_id);
    if (old_iter != old_apps.end())
    {
        app = old_iter->second;
        app->update_from_desktop_file(true);
    }
    else
    {
        app = App::create_from_desktop_id(desktop_id, kind);
        app->signal_launched().connect(sigc::mem_fun(this, &AppManager::app_launched));
        app->signal_close_all_windows().connect(sigc::mem_fun(this, &AppManager::app_close_all_windows));
    }

    this->apps_[desktop_id] = app;

    auto wm_class = app->get_startup_wm_class();
    if (wm_class.length() > 0)
    {
        this->wmclass_apps_[app->get_startup_wm_class()] = app;
    }
}

void AppManager::register_app(std::map<std::string, std::shared_ptr<App>> &old_apps,
                              const std::string &desktop_file,
                              AppKind kind)
{
    std::shared_ptr<App> app = App::create_from_file(desktop_file);
    auto desktop_id = app->get_desktop_id();
    auto old_iter = old_apps.find(desktop_id);
    if (old_iter != old_apps.end())
    {
        app = old_iter->second;
        app->update_from_desktop_file(true);
    }
    else
    {
        app->signal_launched().connect(sigc::mem_fun(this, &AppManager::app_launched));
        app->signal_close_all_windows().connect(sigc::mem_fun(this, &AppManager::app_close_all_windows));
    }

    this->apps_[desktop_id] = app;

    auto wm_class = app->get_startup_wm_class();
    if (wm_class.length() > 0)
    {
        this->wmclass_apps_[app->get_startup_wm_class()] = app;
    }
}

void AppManager::desktop_app_changed(AppManager *app_manager)
{
    KLOG_PROFILE("");

    g_return_if_fail(app_manager == AppManager::get_instance());

    app_manager->load_desktop_apps();

    app_manager->app_desktop_changed_.emit();
}

void AppManager::app_opened(WnckScreen *screen, WnckApplication *wnck_application, gpointer user_data)
{
    auto app_manager = (AppManager *)user_data;

    g_return_if_fail(wnck_application != NULL);
    g_return_if_fail(app_manager == AppManager::get_instance());

    auto xwindow = wnck_application_get_xid(wnck_application);
    auto name = wnck_application_get_name(wnck_application);

    KLOG_DEBUG("wnck app is opened, xid: %" PRIu64 ", name: %s.", xwindow, name);

    auto binding_result = app_manager->bind_wnck2app(wnck_application);

    if (binding_result.first)
    {
        // 如果绑定时创建了新的app，需要发送app安装信号
        if (binding_result.second)
        {
            app_manager->app_installed_.emit(AppVec{binding_result.first});
        }
        app_manager->signal_app_action_changed_.emit(binding_result.first, AppAction::APP_OPENED);
    }
    else
    {
        KLOG_WARNING("Cannot bind wnck application to any app. name: %s xid: %" PRIu64 "\n", name, xwindow);
    }
}

void AppManager::app_closed(WnckScreen *screen, WnckApplication *wnck_application, gpointer user_data)
{
    auto app_manager = (AppManager *)user_data;

    g_return_if_fail(wnck_application != NULL);
    g_return_if_fail(app_manager == AppManager::get_instance());

    auto xwindow = wnck_application_get_xid(wnck_application);
    auto name = wnck_application_get_name(wnck_application);

    KLOG_DEBUG("wnck app is closed, xid: %" PRIu64 ", name: %s.", xwindow, name);

    auto unbinding_result = app_manager->unbind_wnck2app(wnck_application);

    if (unbinding_result.first)
    {
        // 如果取消绑定时删除了fakeapp，需要发送app卸载信号
        if (unbinding_result.second)
        {
            app_manager->app_uninstalled_.emit(AppVec{unbinding_result.first});
        }
        app_manager->signal_app_action_changed_.emit(unbinding_result.first, AppAction::APP_CLOSED);
    }
    else
    {
        KLOG_WARNING("Failed to unbind for wnck application. name: %s xid: %" PRIu64 "\n", name, xwindow);
    }
}

void AppManager::window_opened(std::shared_ptr<Window> window)
{
    KLOG_PROFILE("xid: %" PRIu64 ".", window ? window->get_xid() : 0);

    auto app = lookup_app_with_window(window);
    if (app)
    {
        this->signal_app_action_changed_.emit(app, AppAction::APP_WINDOW_CHANGED);
    }
    else
    {
        KLOG_WARNING("cannot find app for window: %" PRIu64 ".", window->get_xid());
    }
}
void AppManager::window_closed(std::shared_ptr<Window> window)
{
    KLOG_PROFILE("xid: %" PRIu64 ".", window ? window->get_xid() : 0);

    auto app = lookup_app_with_window(window);
    if (app)
    {
        this->signal_app_action_changed_.emit(app, AppAction::APP_WINDOW_CHANGED);
    }
    else
    {
        KLOG_WARNING("cannot find app for window: %" PRIu64 ".", window->get_xid());
    }
}

void AppManager::update_wnckapps_binding()
{
    /* 当出现应用安装时，与之相关的窗口之前是绑定的fake app，此时需要重新绑定；
       当出现应用卸载时，与之相关的窗口失去了绑定，此时需要创建一个fake app来与窗口绑定
    */

    // 先缓存所有的wnckapplication
    std::vector<uint64_t> wnck_apps_xids;
    for (auto &iter : this->wnck_apps_)
    {
        wnck_apps_xids.push_back(iter.first);
    }

    // 清理所有的绑定关系
    this->clear_wnckapps_binding();

    // 重新绑定
    for (auto &wnck_app_xid : wnck_apps_xids)
    {
        auto wnck_application = wnck_application_get(wnck_app_xid);
        // 绑定时如果有新的app创建则交给上一层函数去处理信号发送
        this->bind_wnck2app(wnck_application);
    }
}

std::pair<std::shared_ptr<App>, bool> AppManager::bind_wnck2app(WnckApplication *wnck_application)
{
    RETURN_VAL_IF_TRUE(wnck_application == NULL, std::make_pair(nullptr, false));

    auto xwindow = wnck_application_get_xid(wnck_application);
    auto name = wnck_application_get_name(wnck_application);

    auto iter = this->wnck_apps_.find(xwindow);
    if (iter != this->wnck_apps_.end())
    {
        KLOG_WARNING("The wnck app already exist. name: %s xid: %" PRIu64 "\n", name, xwindow);
        return std::make_pair(nullptr, false);
    }

    auto wnck_windows = wnck_application_get_windows(wnck_application);
    std::shared_ptr<App> app;
    for (auto l = wnck_windows; l != NULL; l = l->next)
    {
        auto wnck_window = (WnckWindow *)(l->data);
        auto window = this->window_manager_->create_temp_window(wnck_window);
        app = this->lookup_app_with_window(window);
        if (app)
        {
            this->wnck_apps_.emplace(xwindow, app);
            app->add_wnck_app_by_xid(xwindow);
            break;
        }
    }

    // 如果现有的app中未找到绑定关系，则创建一个fake app与之绑定
    if (!app)
    {
        app = this->bind_wnck2fake(wnck_application);
        return std::make_pair(app, true);
    }
    return std::make_pair(app, false);
}

std::shared_ptr<App> AppManager::bind_wnck2fake(WnckApplication *wnck_application)
{
    auto xwindow = wnck_application_get_xid(wnck_application);
    auto name = wnck_application_get_name(wnck_application);

    KLOG_DEBUG("create a fake app for wnck app, name: %s xid: %" PRIu64 "\n", name, xwindow);

    auto app = App::create_fake();
    app->add_wnck_app_by_xid(xwindow);

    this->wnck_apps_[xwindow] = app;
    auto &desktop_id = app->get_desktop_id();

    auto iter = this->apps_.emplace(desktop_id, app);
    if (!iter.second)
    {
        KLOG_WARNING("Exist a app that have same desktop id: %s\n", desktop_id.c_str());
        return nullptr;
    }

    return app;
}

std::pair<std::shared_ptr<App>, bool> AppManager::unbind_wnck2app(WnckApplication *wnck_application)
{
    RETURN_VAL_IF_TRUE(wnck_application == NULL, std::make_pair(nullptr, false));

    auto xwindow = wnck_application_get_xid(wnck_application);
    auto name = wnck_application_get_name(wnck_application);

    std::shared_ptr<App> app;
    auto iter = this->wnck_apps_.find(xwindow);
    if (iter == this->wnck_apps_.end())
    {
        KLOG_WARNING("cannot find the App for the wnck_application. name: %s xid: %" PRIu64 "\n", name, xwindow);
        return std::make_pair(nullptr, false);
    }
    else
    {
        app = iter->second.lock();
        if (app)
        {
            app->del_wnck_app_by_xid(xwindow);
            // this->signal_app_action_changed_.emit(app, AppAction::APP_CLOSED);
        }
        this->wnck_apps_.erase(iter);
    }

    if (app &&
        app->get_kind() == AppKind::FAKE_DESKTOP &&
        app->get_wnck_app_count() == 0)
    {
        this->apps_.erase(app->get_desktop_id());
        return std::make_pair(app, true);
    }
    return std::make_pair(app, false);
}

void AppManager::clear_wnckapps_binding()
{
    for (auto &iter : this->wnck_apps_)
    {
        auto app = iter.second.lock();
        if (app)
        {
            app->del_wnck_app_by_xid(iter.first);
        }
    }
    this->wnck_apps_.clear();
}

int32_t AppManager::clear_nouse_fake_app()
{
    int32_t clear_number = 0;
    // 如果fake app不再绑定任何WnckApplication，则进行清理
    for (auto iter = this->apps_.begin(); iter != this->apps_.end();)
    {
        if (iter->second->get_kind() == AppKind::FAKE_DESKTOP &&
            iter->second->get_wnck_app_count() == 0)
        {
            this->apps_.erase(iter++);
            ++clear_number;
            continue;
        }
        ++iter;
    }
    return clear_number;
}

std::string AppManager::get_exec_name(const std::string &exec_str)
{
    auto exec_split = str_split(exec_str, " ");

    if (exec_split.size() == 0)
    {
        return std::string();
    }

    return Glib::path_get_basename(exec_split[0]);
}

void AppManager::app_launched(std::shared_ptr<App> app)
{
    KLOG_PROFILE("app id: %s.", app ? app->get_desktop_id().c_str() : "null");

    this->signal_app_action_changed_.emit(app, AppAction::APP_LAUNCHED);
}

void AppManager::app_close_all_windows(std::shared_ptr<App> app)
{
    KLOG_PROFILE("app id: %s.", app ? app->get_desktop_id().c_str() : "null");

    this->signal_app_action_changed_.emit(app, AppAction::APP_ALL_WINDOWS_CLOSED);
}

}  // namespace Kiran
