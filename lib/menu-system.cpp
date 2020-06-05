/*
 * @Author       : tangjie02
 * @Date         : 2020-04-09 21:42:15
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-04 20:17:09
 * @Description  :
 * @FilePath     : /kiran-menu-2.0/lib/menu-system.cpp
 */
#include "lib/menu-system.h"

#include <libwnck/libwnck.h>

#include "lib/helper.h"
#include "lib/menu-common.h"

namespace Kiran
{
static void monitor_window_open(WnckScreen *screen,
                                WnckWindow *window,
                                gpointer user_data)
{
    MenuSystem *self = (MenuSystem *)user_data;

    auto app = self->lookup_apps_with_window(window);

    if (!app)
    {
        g_debug("not found matching app for open window: %s\n", wnck_window_get_name(window));
        return;
    }

    self->remove_from_new_apps(app);
}

MenuSystem::MenuSystem()
{
    this->settings = Gio::Settings::create(KIRAN_MENU_SCHEMA);

    flush(AppVec());

    read_new_apps();

    WnckScreen *screen = wnck_screen_get_default();
    if (screen)
    {
        wnck_screen_force_update(screen);
        g_signal_connect(screen, "window-opened", G_CALLBACK(monitor_window_open), this);

        // GList *windows = wnck_screen_get_windows(screen);
        // for (GList *l = windows; l != NULL; l = l->next)
        // {
        //     WnckWindow *window = l->data;
        //     GList *match_apps = kiran_menu_system_lookup_apps_with_window(self, window);

        //     g_print("match %u: %s %s\n",
        //             (match_apps != NULL),
        //             wnck_window_get_class_instance_name(window),
        //             wnck_window_get_class_group_name(window));
        // }
    }
    else
    {
        g_warning("the default screen is NULL. please run in GUI application.");
    }
}

MenuSystem::~MenuSystem()
{
    this->apps.clear();
    this->new_apps.clear();
}

void MenuSystem::flush(const AppVec &apps)
{
    gboolean new_app_change = FALSE;

    std::vector<std::shared_ptr<App>> new_installed_apps;
    std::vector<std::shared_ptr<App>> new_uninstalled_apps;

    // copy the keys of the->apps to old_apps.
    auto old_apps = this->apps;

    // update system apps
    this->apps.clear();
    auto registered_apps = Gio::AppInfo::get_all();
    for (auto iter = registered_apps.begin(); iter != registered_apps.end(); ++iter)
    {
        if ((*iter)->should_show())
        {
            auto desktop_id = (*iter)->get_id();
            Glib::Quark quark(desktop_id);
            std::shared_ptr<App> app(new App(desktop_id));
            this->apps[quark.id()] = app;
            app->signal_launched().connect(sigc::mem_fun(this, &MenuSystem::app_launched));
        }
    }

    // new installed apps
    static gboolean first_flush = TRUE;
    if (!first_flush)
    {
        for (auto iter = registered_apps.begin(); iter != registered_apps.end(); ++iter)
        {
            if (!(*iter)->should_show())
            {
                continue;
            }

            auto desktop_id = (*iter)->get_id();
            Glib::Quark quark(desktop_id);

            if (old_apps.find(quark.id()) == old_apps.end())
            {
                auto app = lookup_app(desktop_id);
                new_installed_apps.push_back(app);

                if (std::find(this->new_apps.begin(), this->new_apps.end(), quark.id()) == this->new_apps.end())
                {
                    this->new_apps.push_back(quark.id());
                    new_app_change = TRUE;
                }
            }
        }
    }
    else
    {
        first_flush = FALSE;
    }

    // new uninstalled apps
    {
        for (auto iter = old_apps.begin(); iter != old_apps.end(); ++iter)
        {
            if (this->apps.find(iter->first) == this->apps.end())
            {
                new_uninstalled_apps.push_back(iter->second);
            }
        }
    }

    // uninstalled apps
    {
        auto iter = std::remove_if(this->new_apps.begin(), this->new_apps.end(), [this, &new_app_change](int32_t elem) -> bool {
            if (this->apps.find(elem) == this->apps.end())
            {
                new_app_change = TRUE;
                return true;
            }
            return false;
        });

        if (iter != this->new_apps.end())
        {
            this->new_apps.erase(iter, this->new_apps.end());
        }
    }

    if (new_app_change)
    {
        write_new_apps();
    }

    if (new_installed_apps.size() > 0)
    {
        this->signal_app_installed_.emit(new_installed_apps);
    }

    if (new_uninstalled_apps.size() > 0)
    {
        this->signal_app_uninstalled_.emit(new_uninstalled_apps);
    }
}

std::vector<std::shared_ptr<App>> MenuSystem::get_apps()
{
    std::vector<std::shared_ptr<App>> apps;

    for (auto iter = this->apps.begin(); iter != this->apps.end(); ++iter)
    {
        apps.push_back(iter->second);
    }
    return apps;
}

std::shared_ptr<App> MenuSystem::lookup_app(const std::string &desktop_id)
{
    Glib::Quark quark(desktop_id);

    auto iter = this->apps.find(quark.id());
    if (iter == this->apps.end())
    {
        return nullptr;
    }
    else
    {
        return iter->second;
    }
}

std::shared_ptr<App> MenuSystem::lookup_apps_with_window(WnckWindow *window)
{
    RETURN_VAL_IF_FALSE(window != NULL, nullptr);

    const gchar *instance_name = wnck_window_get_class_instance_name(window);
    const gchar *group_name = wnck_window_get_class_group_name(window);

    typedef enum
    {
        MATCH_NONE,
        MATCH_ALL_ONCE,
        MATCH_ALL_MULTIPLE,
        MATCH_PART_ONCE,
        MATCH_PART_MULTIPLE,
    } WindowMatchType;

    std::shared_ptr<App> match_app;
    WindowMatchType match_type = MATCH_NONE;
    for (auto iter = this->apps.begin(); iter != this->apps.end(); ++iter)
    {
        auto &app = iter->second;
        auto &exec = app->get_exec();
        auto &locale_name = app->get_locale_name();

        g_autofree gchar *exec_name = get_exec_name(exec.c_str());

        //g_print("exec_name: %s instance_name: %s group_name: %s locale_name: %s\n", exec_name, instance_name, group_name, locale_name.c_str());

        gboolean match_instance_name = (g_strcmp0(exec_name, instance_name) == 0);
        gboolean match_group_name = (g_strcmp0(group_name, locale_name.c_str()) == 0);

        if (match_instance_name && match_group_name)
        {
            if (match_type == MATCH_ALL_ONCE || match_type == MATCH_ALL_MULTIPLE)
            {
                match_type = MATCH_ALL_MULTIPLE;
            }
            else
            {
                match_app = app;
                match_type = MATCH_ALL_ONCE;
            }
        }
        else if ((match_instance_name || match_group_name) &&
                 match_type != MATCH_ALL_ONCE &&
                 match_type != MATCH_ALL_MULTIPLE)
        {
            if (match_type == MATCH_PART_ONCE || match_type == MATCH_PART_MULTIPLE)
            {
                match_type = MATCH_PART_MULTIPLE;
            }
            else
            {
                match_app = app;
                match_type = MATCH_PART_ONCE;
            }
        }
    }
    if (match_type == MATCH_ALL_MULTIPLE)
    {
        g_warning("Multiple App match the window in terms of instance name and group name.");
    }
    else if (match_type == MATCH_PART_MULTIPLE)
    {
        g_warning("Multiple App match the window in terms of instance name or group name.");
    }

    return match_app;
}

std::vector<std::string> MenuSystem::get_nnew_apps(gint top_n)
{
    std::vector<std::string> new_apps;

    for (auto iter = this->new_apps.begin(); iter != this->new_apps.end(); ++iter)
    {
        Glib::QueryQuark query_quark((GQuark)(*iter));
        Glib::ustring desktop_id = query_quark;
        new_apps.push_back(desktop_id);
    }

    if (top_n > 0 && top_n < new_apps.size())
    {
        new_apps.resize(top_n);
    }

    return new_apps;
}

std::vector<std::string> MenuSystem::get_all_sorted_apps()
{
    std::vector<std::string> apps;

    for (auto iter = this->apps.begin(); iter != this->apps.end(); ++iter)
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

void MenuSystem::remove_from_new_apps(std::shared_ptr<App> app)
{
    auto &desktop_id = app->get_desktop_id();
    Glib::Quark quark(desktop_id);
    int32_t remove_value = quark.id();

    auto iter = std::remove_if(this->new_apps.begin(), this->new_apps.end(), [remove_value](int32_t elem) {
        return elem == remove_value;
    });

    if (iter != this->new_apps.end())
    {
        this->new_apps.erase(iter, this->new_apps.end());
        write_new_apps();
    }
}

gchar *MenuSystem::get_exec_name(const gchar *exec_str)
{
    RETURN_VAL_IF_FALSE(exec_str != NULL, NULL);

    g_auto(GStrv) exec_split = g_strsplit(exec_str, " ", -1);

    if (!exec_split || !(exec_split[0]))
    {
        return NULL;
    }

    gchar *exec_name = NULL;

    exec_name = g_path_get_basename(exec_split[0]);
    if (g_strcmp0(exec_name, "flatpak") == 0)
    {
        g_free(exec_name);
        for (gint i = 0; exec_split[i] != NULL; ++i)
        {
            if (g_str_has_prefix(exec_split[i], "--command="))
            {
                g_auto(GStrv) command_split = g_strsplit(exec_split[i], "=", -1);
                exec_name = g_path_get_basename(command_split[1]);
            }
        }
    }
    return exec_name;
}

void MenuSystem::read_new_apps()
{
    this->new_apps.clear();
    this->new_apps = read_as_to_list_quark(this->settings, "new-apps");
}

void MenuSystem::write_new_apps()
{
    write_list_quark_to_as(this->settings, "new-apps", this->new_apps);
    this->signal_new_app_changed_.emit();
}

void MenuSystem::app_launched(std::shared_ptr<App> app)
{
    remove_from_new_apps(app);
}

}  // namespace Kiran