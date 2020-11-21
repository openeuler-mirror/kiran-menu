#include "menu-applet-window.h"
#include "menu-app-launcher-button.h"
#include "menu-power-button.h"
#include "kiran-search-entry.h"
#include "kiran-helper.h"
#include "menu-avatar-widget.h"
#include "global.h"
#include <gtk/gtkx.h>
#include "window-manager.h"


#include <unistd.h>
#include <iostream>
#include <glibmm/i18n.h>
#include "global.h"

#define NEW_APPS_MAX_SIZE 3

MenuAppletWindow::MenuAppletWindow(Gtk::WindowType window_type):
    Glib::ObjectBase("KiranMenuWindow"),
    Gtk::Window(window_type),
    compact_min_height_property(*this, "compact-min-height", 0),
    expand_min_height_property(*this, "expand-min-height", 0),
    compact_apps_button(nullptr),
    compact_favorites_button(nullptr)
{

    set_name("menu-window");
    set_skip_taskbar_hint(true);
    set_skip_pager_hint(true);
    set_decorated(false);
    set_accept_focus(true);
    set_focus_on_map(true);

    init_ui();

    Kiran::WindowManager::get_instance()->signal_active_window_changed().connect(
                sigc::hide<0>(sigc::mem_fun(*this, &MenuAppletWindow::on_active_window_changed)));

    auto backend = Kiran::MenuSkeleton::get_instance();
    backend->signal_app_changed().connect(sigc::mem_fun(*this, &MenuAppletWindow::reload_apps_data));
    backend->signal_favorite_app_added().connect(sigc::hide(sigc::mem_fun(*this, &MenuAppletWindow::load_favorite_apps)));
    backend->signal_favorite_app_deleted().connect(sigc::hide(sigc::mem_fun(*this, &MenuAppletWindow::load_favorite_apps)));
    backend->signal_frequent_usage_app_changed().connect(sigc::mem_fun(*this, &MenuAppletWindow::load_frequent_apps));
    backend->signal_new_app_changed().connect(sigc::mem_fun(*this, &MenuAppletWindow::load_new_apps));

    profile.signal_changed().connect(sigc::mem_fun(*this, &MenuAppletWindow::on_profile_changed));
    property_is_active().signal_changed().connect(sigc::mem_fun(*this, &MenuAppletWindow::on_active_change));

    auto screen = get_screen();
    monitor = new WorkareaMonitor(screen);
    monitor->signal_size_changed().connect(sigc::mem_fun(*this, &MenuAppletWindow::ensure_display_mode));

    //加载当前用户信息
    user_info = new KiranUserInfo(getuid());
    set_display_mode(profile.get_display_mode());
}

MenuAppletWindow::~MenuAppletWindow()
{
    delete user_info;
    delete monitor;
}

sigc::signal<void, int, int> MenuAppletWindow::signal_size_changed()
{
    return m_signal_size_changed;
}

void MenuAppletWindow::reload_apps_data()
{
    load_new_apps();
    load_frequent_apps();
    load_all_apps();
    load_favorite_apps();
}

void MenuAppletWindow::ensure_display_mode()
{
    set_display_mode(profile.get_display_mode());
}

void MenuAppletWindow::on_realize()
{

    Gtk::Window::on_realize();
}

void MenuAppletWindow::on_unrealize()
{
    delete monitor;
    Gtk::Window::on_unrealize();
}

void MenuAppletWindow::get_preferred_height_vfunc(int &min_height, int &natural_height) const
{
    int min_height_from_css;
    Glib::RefPtr<const Gdk::Monitor> monitor;
    Gdk::Rectangle workarea;

    monitor = Gdk::Display::get_default()->get_primary_monitor();
    monitor->get_workarea(workarea);

    Gtk::Window::get_preferred_height_vfunc(min_height, natural_height);

    if (display_mode == DISPLAY_MODE_EXPAND)
        min_height_from_css = expand_min_height_property.get_value();
    else
        min_height_from_css = compact_min_height_property.get_value();

    /* 最小高度不能超过屏幕高度 */
    min_height = std::max(min_height_from_css, min_height);
    min_height = std::min(workarea.get_height(), min_height);

    /* 自然高度不能小于最小高度 */
    natural_height = std::max(min_height, natural_height);
}

void MenuAppletWindow::on_active_change()
{
    if (is_active())
        KiranHelper::grab_input(*this);
}

bool MenuAppletWindow::on_draw(const Cairo::RefPtr<Cairo::Context> &cr)
{
    double opacity;
    Gdk::RGBA background_color;
    Gtk::Widget *child;
    Gtk::Allocation allocation;
    auto context = get_style_context();

    allocation = get_allocation();
    background_color = context->get_background_color(get_state_flags());
    opacity = profile.get_opacity();
    cr->save();

    background_color.set_alpha(opacity);
    Gdk::Cairo::set_source_rgba(cr, background_color);
    cr->set_operator(Cairo::OPERATOR_SOURCE);

    cr->paint();
    cr->restore();

    context->render_frame(cr, 0, 0, allocation.get_width(), allocation.get_height());

    child = get_child();
    propagate_draw(*child, cr);
    return true;
}

void MenuAppletWindow::init_ui()
{
    Gtk::Box *search_box;
    Gtk::EventBox *date_box;

    init_window_visual();

    builder = Gtk::Builder::create_from_resource("/kiran-menu/ui/menu");
    builder->get_widget<Gtk::Box>("menu-container", box);
    builder->get_widget<Gtk::Grid>("sidebar-box", side_box);
    builder->get_widget<Gtk::Stack>("overview-stack", overview_stack);
    builder->get_widget<Gtk::Stack>("apps-view-stack", appview_stack);

    builder->get_widget<Gtk::Box>("new-apps-box", new_apps_box);
    builder->get_widget<Gtk::Box>("all-apps-box", all_apps_box);
    builder->get_widget<Gtk::Grid>("category-overview-box", category_overview_box);
    builder->get_widget<Gtk::Grid>("search-results-box", search_results_box);
    builder->get_widget<Gtk::Box>("compact-tab-box", compact_tab_box);
    builder->get_widget<Gtk::Box>("search-box", search_box);
    builder->get_widget<Gtk::EventBox>("date-box", date_box);

    init_scrollable_areas();

    category_overview_box->set_orientation(Gtk::ORIENTATION_VERTICAL);

    //添加搜索框
    search_entry = create_app_search_entry();
    search_box->add(*search_entry);

    date_box->add_events(Gdk::BUTTON_PRESS_MASK | Gdk::ENTER_NOTIFY_MASK | Gdk::LEAVE_NOTIFY_MASK);
    date_box->signal_button_press_event().connect_notify(
                sigc::hide(
                    sigc::mem_fun(*this, &MenuAppletWindow::on_date_box_clicked)));

    //添加侧边栏应用快捷方式
    add_sidebar_buttons();

    //通过reparent，将布局添加到当前窗口
    box->reparent(*this);
    box->show_all();
}

void MenuAppletWindow::init_scrollable_areas()
{
    Gtk::Viewport *viewport;
    std::vector<Gtk::Viewport*> viewport_list;


    builder->get_widget<Gtk::Viewport>("all-apps-viewport", viewport);
    viewport_list.push_back(viewport);
    builder->get_widget<Gtk::Viewport>("search-results-viewport", viewport);
    viewport_list.push_back(viewport);
    builder->get_widget<Gtk::Viewport>("category-overview-viewport", viewport);
    viewport_list.push_back(viewport);
    builder->get_widget<Gtk::Viewport>("compact-favorites-viewport", viewport);
    viewport_list.push_back(viewport);
    builder->get_widget<Gtk::Viewport>("expand-favorites-viewport", viewport);
    viewport_list.push_back(viewport);

    /* 确保app列表中的应用和分类获取焦点时通过滚动保证可见 */
    for (auto viewport: viewport_list)
        viewport->signal_realize().connect(
            [viewport]() -> void {
                if (!viewport)
                    return;

                auto scrolled_window = dynamic_cast<Gtk::ScrolledWindow *>(viewport->get_parent());
                viewport->set_focus_vadjustment(scrolled_window->get_vadjustment());
            });
}

void MenuAppletWindow::on_active_window_changed(std::shared_ptr<Kiran::Window> active_window)
{
    if (!get_realized())
        return;

    if (!active_window || active_window->get_xid() != GDK_WINDOW_XID(get_window()->gobj()))
        hide();
}


Gtk::SearchEntry *MenuAppletWindow::create_app_search_entry()
{
    auto search_entry = Gtk::make_managed<KiranSearchEntry>();
    search_entry->set_can_default(true);
    search_entry->set_can_focus(true);
    search_entry->set_activates_default(true);

    search_entry->signal_search_changed().connect(sigc::mem_fun(*this, &MenuAppletWindow::on_search_change));
    search_entry->signal_stop_search().connect(sigc::mem_fun(*this, &MenuAppletWindow::on_search_stop));
    search_entry->signal_activate().connect(sigc::mem_fun(*this, &MenuAppletWindow::activate_search_result));

    return search_entry;
}

void MenuAppletWindow::on_date_box_clicked()
{
    const char *app_names[] = {
        "kiran-timedate-manager",
        "mate-time-admin",
        "system-config-date",
        nullptr};

    if (!launch_app_from_list(app_names))
        g_warning("Failed to launch datetime manage tools");

    hide();
}

void MenuAppletWindow::on_avatar_clicked()
{
    const char *app_names[] = {
        "kiran-account-manager",
        "mate-about-me",
        "system-config-users",
        nullptr};

    if (!launch_app_from_list(app_names))
        g_warning("Failed to launch avatar or account manage tools");

    hide();
}

void MenuAppletWindow::on_profile_changed(const Glib::ustring &changed_key)
{
    if (changed_key != "opacity")
        set_display_mode(profile.get_display_mode());
    else
        queue_draw();
}

void MenuAppletWindow::init_window_visual()
{
    Glib::RefPtr<Gdk::Visual> rgba_visual;
    GtkWidget *widget = GTK_WIDGET(gobj());

    /*设置窗口的Visual为RGBA visual，确保窗口背景透明度可以正常绘制 */
    rgba_visual = get_screen()->get_rgba_visual();
    gtk_widget_set_visual(widget, rgba_visual->gobj());
}

void MenuAppletWindow::on_search_change()
{
    if (search_entry->get_text_length() == 0) {
        //搜索内容为空，返回到应用列表页面
        on_search_stop();
        return;
    }

    //清空之前的搜索结果
    search_results_box->remove_data("first-item");
    KiranHelper::remove_all_for_container(*search_results_box);

    //切换到搜索结果页面
    set_stack_current_index(appview_stack, PAGE_SEARCH_RESULT, false);

    //搜索时忽略关键字大小写
    auto apps_list = Kiran::MenuSkeleton::get_instance()->search_app(search_entry->get_text().data(), true);
    if (apps_list.size()) {
        g_debug("search results length %lu\n", apps_list.size());
        auto category_item = Gtk::make_managed<MenuCategoryItem>(_("Search Results"), false);

        search_results_box->add(*category_item);
        for (auto iter = apps_list.begin(); iter != apps_list.end(); iter++) {
            auto app_item = create_app_item(*iter);

            search_results_box->add(*app_item);
            if (iter == apps_list.begin())
                search_results_box->set_data("first-item", app_item);
        }
    } else {
        //搜索结果为空
        Gtk::Label *label;

        label = create_empty_prompt_label(_("No matched apps found!"));
        search_results_box->add(*label);
    }

    search_results_box->show_all();
}

void MenuAppletWindow::activate_search_result()
{
    MenuAppItem *item;

    if (get_stack_current_index(appview_stack) != PAGE_SEARCH_RESULT)
        return;

    if (!search_entry->has_focus())
        return;

    item = reinterpret_cast<MenuAppItem*>(search_results_box->get_data("first-item"));
    if (item != nullptr)
        item->launch_app();
}

int MenuAppletWindow::get_stack_current_index(Gtk::Stack *stack)
{
    Gtk::Widget *child = stack->get_visible_child();

    return stack->child_property_position(*child).get_value();
}

void MenuAppletWindow::set_stack_current_index(Gtk::Stack *stack, int index, bool animation)
{
    for (auto child: stack->get_children()) {
        int pos;

        pos = stack->child_property_position(*child).get_value();
        if (pos == index) {
            stack->set_visible_child(*child);
            break;
        }
    }
}

void MenuAppletWindow::on_search_stop()
{
    /*返回应用列表页面，并清空搜索内容*/
    set_stack_current_index(appview_stack, PAGE_ALL_APPS_LIST, false);
    search_entry->set_text("");
}


void MenuAppletWindow::switch_to_category_overview(const std::string &selected_category)
{
    MenuCategoryItem *selected_item = nullptr;

    KiranHelper::remove_all_for_container(*category_overview_box);


    for (auto iter = category_names.begin(); iter != category_names.end(); iter++) {
        auto item = Gtk::make_managed<MenuCategoryItem>(*iter, true);
        item->set_hexpand(true);
        item->show_all();

        if (item->get_category_name() == selected_category) {
            g_debug("found target category: '%s'", item->get_category_name().c_str());
            selected_item = item;
        }
        item->signal_clicked().connect(sigc::bind<const std::string&, bool>(
                                           sigc::mem_fun<const std::string&>(*this, &MenuAppletWindow::switch_to_apps_overview),
                                           item->get_category_name(),
                                           true));
        category_overview_box->add(*item);
    }
    set_stack_current_index(overview_stack, VIEW_CATEGORY_SELECTION, true);

    if (selected_item)
        selected_item->grab_focus();
}


void MenuAppletWindow::switch_to_apps_overview(const std::string &selected_category,
                                              bool animation)
{
    MenuCategoryItem *item;
    Gtk::Allocation allocation;
    double adjusted_pos = -1;


    if (!selected_category.empty()) {

        //找到分类标签对应的控件
        auto iter = category_items.find(selected_category);
        if (iter != category_items.end()) {
            std::pair<std::string, MenuCategoryItem*> data = *iter;
            item = data.second;

            //滚动到指定的分类标签
            allocation = item->get_allocation();
            adjusted_pos = allocation.get_y();
            switch_to_apps_overview(adjusted_pos, animation);

            //将指定的分类标签控件添加焦点
            g_debug("grab focus for category '%s'\n", selected_category.c_str());
            item->grab_focus();
        } else
            g_warning("invalid category name: '%s'\n", selected_category.c_str());
    }
}

void MenuAppletWindow::switch_to_apps_overview(double position, bool animation)
{
    Gtk::ScrolledWindow *all_apps_area;

    //切换到应用程序列表
    set_stack_current_index(overview_stack, VIEW_APPS_LIST, animation);
    if (position >= 0) {
        builder->get_widget<Gtk::ScrolledWindow>("all-apps-container", all_apps_area);

        auto adjustment = all_apps_area->get_vadjustment();
        adjustment->set_value(position);
    }

}

bool MenuAppletWindow::launch_app_from_list(const char **app_names)
{
    std::shared_ptr<Kiran::App> app;
    auto app_manager = Kiran::AppManager::get_instance();

    for (int i = 0; app_names[i] != nullptr; i++) {
        app = app_manager->lookup_app(std::string(app_names[i]) + ".desktop");
        if (app)
            break;
    }

    if (app) {
        app->launch();
        return true;
    }

    return false;
}

bool MenuAppletWindow::on_map_event(GdkEventAny *any_event)
{
    Gtk::Window::on_map_event(any_event);

    /**
     * 获取当前系统的鼠标事件，这样才能在鼠标点击窗口外部时及时隐藏窗口
     */
    KiranHelper::grab_input(*this);

    //gtk_grab_add(GTK_WIDGET(this->gobj()));

    //应用列表滚动到开始位置
    switch_to_apps_overview(0, false);

    on_search_stop();

    if (display_mode == DISPLAY_MODE_EXPAND || profile.get_default_page() == PAGE_ALL_APPS)
        search_entry->grab_focus();
    else
        compact_favorites_button->clicked();
    return true;
}

bool MenuAppletWindow::on_unmap_event(GdkEventAny *any_event)
{
    KiranHelper::ungrab_input(*this);
    //gtk_grab_remove(GTK_WIDGET(this->gobj()));
    return Gtk::Window::on_unmap_event(any_event);
}

bool MenuAppletWindow::on_button_press_event(GdkEventButton *event)
{
    int window_x, window_y;
    int window_width, window_height;

    get_window()->get_root_origin(window_x, window_y);
    window_width = get_window()->get_width();
    window_height = get_window()->get_height();

    if (((int)event->x_root < window_x || (int)event->x_root > window_x + window_width) ||
        ((int)event->y_root < window_y || (int)event->y_root > window_y + window_height)) {
        //点击位置在菜单窗口之外，隐藏窗口
        hide();
    }

    return false;
}

bool MenuAppletWindow::on_configure_event(GdkEventConfigure *configure_event)
{
    /**
     * 缓存窗口当前位置和尺寸，以检查是否是大小发生变化
     * 如果尺寸发生变化，发出信号通知
     */
    if (get_mapped()) {
        if (geometry.get_width() != configure_event->width ||
                geometry.get_height() != configure_event->height)
            m_signal_size_changed.emit(configure_event->width, configure_event->height);
    }

    geometry.set_x(configure_event->x);
    geometry.set_y(configure_event->y);
    geometry.set_width(configure_event->width);
    geometry.set_height(configure_event->height);

    return Gtk::Window::on_configure_event(configure_event);
}


bool MenuAppletWindow::on_key_press_event(GdkEventKey *key_event)
{
    if (key_event->keyval == GDK_KEY_Escape) {
        switch (get_stack_current_index(overview_stack)) {
        case VIEW_APPS_LIST:
            /**
             * 当前位于应用列表视图.
             * 无搜索结果时，按下ESC键隐藏开始菜单窗口.否则停止搜索。
             */

            if (get_stack_current_index(appview_stack) != PAGE_SEARCH_RESULT)
                hide();
            else
                on_search_stop();
            return true;
        case VIEW_CATEGORY_SELECTION:
            /*当前处于分类选择视图, 返回应用视图*/
            switch_to_apps_overview(-1, false);
            return true;
        default:
            hide();
            return true;
        }

    } else if (key_event->keyval == GDK_KEY_Super_L || key_event->keyval == GDK_KEY_Super_R) {
        hide();
        return true;
    } else {
        if (search_entry->handle_event(key_event) == GDK_EVENT_STOP) {
            /**
             * 优先搜索框处理，同时让搜索框获取焦点
             * 搜索框获取焦点时，原来的输入框内容会被选中,
             * 因此需要调用select_region来取消选中
             */
            if (!search_entry->is_focus()) {
                search_entry->grab_focus();
                search_entry->select_region(-1, -1);
            }
            return true;
        }
    }

    //传递给子控件处理
    return Gtk::Window::on_key_press_event(key_event);
}

bool MenuAppletWindow::on_leave_notify_event(GdkEventCrossing *crossing_event)
{
    return false;
}

Gtk::Button *MenuAppletWindow::create_launcher_button(const char *icon_resource,
                                     const char *tooltip,
                                     const char *cmdline)
{
    MenuAppLauncherButton *button;

    button = Gtk::make_managed<MenuAppLauncherButton>(icon_resource,
                                                     tooltip,
                                                     cmdline);

    button->signal_app_launched().connect(sigc::mem_fun(*this, &Gtk::Widget::hide));
    return button;
}

Gtk::Button* MenuAppletWindow::create_page_button(const char *icon_resource,
                                                 const char *tooltip,
                                                 int page_index)
{
    Gtk::Button *button = Gtk::make_managed<Gtk::Button>();
    Gtk::Image *image = Gtk::make_managed<Gtk::Image>();

    try {
        image->set_pixel_size(16);
        image->set_from_resource(icon_resource);
    } catch (const Glib::Error &e) {
        g_warning("Failed to load resouce '%s': %s", icon_resource, e.what().c_str());
    }

    button->set_always_show_image(true);
    button->set_image(*image);
    button->set_tooltip_text(tooltip);
    button->get_style_context()->add_class("kiran-app-button");

    button->signal_clicked().connect(
                [this, page_index]() -> void{
                    set_stack_current_index(overview_stack, page_index, true);
                    search_entry->grab_focus();
                });

    return button;
}

Gtk::Label *MenuAppletWindow::create_empty_prompt_label(const char *prompt_text)
{
    Gtk::Label *label = nullptr;

    label = Gtk::make_managed<Gtk::Label>(prompt_text);
    label->get_style_context()->add_class("search-empty-prompt");
    label->set_hexpand(true);
    label->set_vexpand(true);
    label->set_halign(Gtk::ALIGN_CENTER);
    label->set_valign(Gtk::ALIGN_CENTER);
    return label;
}

void MenuAppletWindow::add_sidebar_buttons()
{
    Gtk::Separator *separator;
    Gtk::Button *power_btn, *launcher_btn;

    separator= Gtk::make_managed<Gtk::Separator>(Gtk::ORIENTATION_HORIZONTAL);
    separator->set_margin_start(9);
    separator->set_margin_end(9);
    separator->set_margin_top(5);
    separator->set_margin_bottom(5);

    power_btn = Gtk::make_managed<MenuPowerButton>();
    side_box->set_orientation(Gtk::ORIENTATION_VERTICAL);

    compact_favorites_button = create_page_button("/kiran-menu/icon/favorite",
                                                  _("Favorites"),
                                                  VIEW_COMPACT_FAVORITES);
    compact_apps_button = create_page_button("/kiran-menu/icon/list",
                                             _("All applications"),
                                             VIEW_APPS_LIST);

    compact_tab_box->add(*compact_favorites_button);
    compact_tab_box->add(*compact_apps_button);
    compact_tab_box->add(*separator);

    launcher_btn = create_launcher_button("/kiran-menu/sidebar/home-dir",
                                          _("Home Directory"),
                                          "caja");
    side_box->add(*launcher_btn);

    launcher_btn = create_launcher_button("/kiran-menu/sidebar/settings",
                                          _("Control center"),
                                          "mate-control-center");
    side_box->add(*launcher_btn);

    launcher_btn = create_launcher_button("/kiran-menu/sidebar/monitor",
                                  _("System monitor"),
                                  "mate-system-monitor");
    side_box->add(*launcher_btn);

    launcher_btn = create_launcher_button("/kiran-menu/sidebar/lock",
                                          _("Lock Screen"),
                                          "mate-screensaver-command -l");
    side_box->add(*launcher_btn);

    side_box->add(*power_btn);

    side_box->show_all();
}

void MenuAppletWindow::load_all_apps()
{
    Gtk::Box *all_apps_box;
    auto backend = Kiran::MenuSkeleton::get_instance();

    builder->get_widget<Gtk::Box>("all-apps-box", all_apps_box);

    //清空原有的应用和分类信息
    category_items.clear();
    KiranHelper::remove_all_for_container(*all_apps_box);

    //删除空的应用分类
    category_names = Kiran::MenuSkeleton::get_instance()->get_category_names();
    auto start = std::remove_if(category_names.begin(), category_names.end(),
            [](const std::string &name) -> bool {
                int apps_size;

                apps_size = Kiran::MenuSkeleton::get_instance()->get_category_apps(name).size();
                if (apps_size == 0)
                    g_debug("%s: Remove empty category '%s'\n", __FUNCTION__, name.data());
                return (apps_size == 0);
		    });
    category_names.erase(start, category_names.end());

    //遍历分类列表，建立应用列表
    for (auto category: category_names) {
        auto apps_list = backend->get_category_apps(category);
        auto item = create_category_item(category);

        category_items.insert(category_items.end(), std::pair<std::string, MenuCategoryItem*>(category, item));
        item->signal_clicked().connect(sigc::bind<const std::string&>(
                                           sigc::mem_fun(*this, &MenuAppletWindow::switch_to_category_overview),
                                           item->get_category_name()));
        all_apps_box->add(*item);
        for (auto app: apps_list) {
            auto item = create_app_item(app);
            all_apps_box->add(*item);
        }
    }

    all_apps_box->show_all();
}

void MenuAppletWindow::load_frequent_apps()
{
    MenuListItemWidget *item;
    auto apps_list = Kiran::MenuSkeleton::get_instance()->get_nfrequent_apps(4);

    if (display_mode == DISPLAY_MODE_COMPACT) {
        //TODO
        Gtk::Box *frequent_apps_box;

        builder->get_widget<Gtk::Box>("compact-frequent-box", frequent_apps_box);
        KiranHelper::remove_all_for_container(*frequent_apps_box);

        if (apps_list.size() > 0) {
            item = create_category_item(_("Frequently used"), false);
            frequent_apps_box->add(*item);

            for (auto app: apps_list) {
                item = create_app_item(app, Gtk::ORIENTATION_HORIZONTAL);
                frequent_apps_box->add(*item);
            }
        }
        frequent_apps_box->show_all();
    } else {
        Gtk::Box *frequent_header_box, *frequent_container;
        Gtk::FlowBox *frequent_apps_box;

        builder->get_widget<Gtk::FlowBox>("expand-frequent-box", frequent_apps_box);
        builder->get_widget<Gtk::Box>("expand-frequent-header", frequent_header_box);
        builder->get_widget<Gtk::Box>("expand-frequent-container", frequent_container);
        frequent_apps_box->set_orientation(Gtk::ORIENTATION_HORIZONTAL);

        KiranHelper::remove_all_for_container(*frequent_apps_box);
        KiranHelper::remove_all_for_container(*frequent_header_box);

        if (apps_list.size() > 0) {
            item = create_category_item(_("Frequently used"), false);
            frequent_header_box->add(*item);
            for (auto app: apps_list) {
                item = create_app_item(app, Gtk::ORIENTATION_VERTICAL);
                frequent_apps_box->insert(*item, -1);
            }
            frequent_container->show_all();
        } else {
            frequent_container->hide();
        }
    }
}

void MenuAppletWindow::load_new_apps()
{
    Gtk::Box *new_apps_box;
    builder->get_widget<Gtk::Box>("new-apps-box", new_apps_box);

    KiranHelper::remove_all_for_container(*new_apps_box);

    //加载新安装应用列表
    auto new_apps_list = Kiran::MenuSkeleton::get_instance()->get_nnew_apps(-1);
    if (new_apps_list.size() > 0) {
        int index = 1;
        Gtk::Box *more_apps_box = nullptr;
        Gtk::ToggleButton *expand_button = nullptr;
        auto item = create_category_item(_("New Installed"), false);

        new_apps_box->add(*item);
        for (auto app: new_apps_list) {
            auto item = create_app_item(app);

            if (index <= NEW_APPS_MAX_SIZE)
                new_apps_box->add(*item);
            else {
                //新安装应用数量多，只显示部分应用和展开按钮
                if (!more_apps_box) {
                    auto image = Gtk::make_managed<Gtk::Image>();
                    image->set_from_resource("/kiran-menu/icon/expand");

                    expand_button = Gtk::make_managed<Gtk::ToggleButton>(_("Expand"));
                    more_apps_box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL);
                    new_apps_box->add(*expand_button);
                    new_apps_box->add(*more_apps_box);

                    expand_button->get_style_context()->add_class("kiran-expand-button");
                    expand_button->set_image(*image);
                    expand_button->set_image_position(Gtk::POS_RIGHT);
                    expand_button->set_alignment(0.0, 0.5);
                    expand_button->signal_toggled().connect(
                                [expand_button, more_apps_box, image]() -> void {
                                    more_apps_box->set_visible(expand_button->get_active());
                                    expand_button->set_label(expand_button->get_active()?_("Shrink"):_("Expand"));
                                    image->set_from_resource(
                                                expand_button->get_active()?"/kiran-menu/icon/shrink":"/kiran-menu/icon/expand");
                                });
                }
                more_apps_box->add(*item);
            }
            index++;
        }
        new_apps_box->show_all();
        if (more_apps_box)
            more_apps_box->set_visible(false);
    }
}

void MenuAppletWindow::load_favorite_apps()
{
    MenuListItemWidget *item;
    Gtk::Box *favorite_header_box;
    Gtk::FlowBox *favorite_apps_box;
    auto apps_list = Kiran::MenuSkeleton::get_instance()->get_favorite_apps();

    if (display_mode == DISPLAY_MODE_COMPACT) {
        builder->get_widget<Gtk::FlowBox>("compact-favorites-box", favorite_apps_box);
        builder->get_widget<Gtk::Box>("compact-favorites-header", favorite_header_box);
    } else {
        builder->get_widget<Gtk::FlowBox>("expand-favorite-box", favorite_apps_box);
        builder->get_widget<Gtk::Box>("expand-favorite-header", favorite_header_box);
    }

    KiranHelper::remove_all_for_container(*favorite_apps_box);
    KiranHelper::remove_all_for_container(*favorite_header_box);

    if (apps_list.size() == 0) {
        Gtk::Label *label;

        label = create_empty_prompt_label(_("No favorite apps!"));
        favorite_apps_box->add(*label);
    } else {
        int index = 0;
        item = create_category_item(_("Favorite Apps"), false);

        favorite_header_box->add(*item);

        for (auto app: apps_list) {
            if (display_mode == DISPLAY_MODE_EXPAND) {
                item = create_app_item(app, Gtk::ORIENTATION_VERTICAL);
                item->set_vexpand(false);
                item->set_valign(Gtk::ALIGN_START);
                favorite_apps_box->insert(*item, -1);
                index++;
            } else {
                item = create_app_item(app);
                item->set_hexpand(true);
                item->set_halign(Gtk::ALIGN_FILL);
                favorite_apps_box->insert(*item, -1);
            }
        }
    }

    favorite_header_box->show_all();
    favorite_apps_box->show_all();

}

void MenuAppletWindow::load_user_info()
{

    Gtk::Box *avatar_box;
    MenuAvatarWidget *avatar;
    int icon_size = 60;

    if (!user_info->is_ready())
        return;

    if (display_mode == DISPLAY_MODE_COMPACT)
        icon_size = 36;

    avatar = Gtk::make_managed<MenuAvatarWidget>(icon_size);
    avatar->set_icon(user_info->get_iconfile());
    avatar->signal_button_press_event().connect_notify(sigc::hide(sigc::mem_fun(*this, &MenuAppletWindow::on_avatar_clicked)));


    if (display_mode == DISPLAY_MODE_COMPACT) {
        //显示用户头像并在tooltip中提示用户名
        builder->get_widget("compact-avatar-box", avatar_box);
        avatar->set_tooltip_text(Glib::ustring(user_info->get_username()));
    } else {
        Gtk::Label *name_label;
        builder->get_widget("expand-avatar-box", avatar_box);
        builder->get_widget<Gtk::Label>("username-label", name_label);

        avatar->set_vexpand(true);
        name_label->set_markup(Glib::ustring(_("Hello")) + ", <b>" + user_info->get_username() +"</b>");
    }
    KiranHelper::remove_all_for_container(*avatar_box);
    avatar_box->add(*avatar);
    avatar_box->show_all();
}

void MenuAppletWindow::set_display_mode(MenuDisplayMode mode)
{
    Gtk::Box *avatar_button;
    Gtk::Box *expand_panel, *compact_tab_box;
    int min_width, natural_width, min_height, natural_height;
    Gdk::Rectangle rect;
    Glib::RefPtr<Gdk::Monitor> monitor;
    Glib::RefPtr<Gdk::Display> display;

    if (get_realized()) {
        display = get_display();
    } else
        display = Gdk::Display::get_default();

    display_mode = mode;
    monitor = display->get_primary_monitor();
    monitor->get_workarea(rect);

    builder->get_widget("compact-avatar-box", avatar_button);
    builder->get_widget("expand-panel", expand_panel);
    builder->get_widget("compact-tab-box", compact_tab_box);

    if(display_mode == DISPLAY_MODE_COMPACT) {
        //紧凑模式下隐藏右侧面板
        avatar_button->set_visible(true);
        expand_panel->set_visible(false);
        compact_tab_box->set_visible(true);

        if (profile.get_default_page() == PAGE_ALL_APPS)
            compact_apps_button->clicked();
        else
            compact_favorites_button->clicked();
    } else {

        avatar_button->set_visible(false);
        expand_panel->set_visible(true);
        compact_tab_box->set_visible(false);
        get_preferred_width(min_width, natural_width);
        g_debug("min-width: %d, workarea %d x %d", min_width,
                rect.get_width(),
                rect.get_height());

        if (min_width > rect.get_width()) {
            /*当前屏幕宽度小于扩展模式的最小宽度，切换到紧凑模式*/
            g_warning("%s: min width for expand mode exceeds monitor size, switch to compact mode now\n", __func__);
            set_display_mode(DISPLAY_MODE_COMPACT);
            return;
        }

        load_date_info();
    }
    get_preferred_width(min_width, natural_width);
    get_preferred_height(min_height, natural_height);
    /* 等待DBus信息准备好 */
    if (!user_info->is_ready()) {
        user_info->signal_data_ready().clear();
        user_info->signal_data_ready().connect(
                    sigc::mem_fun(*this, &MenuAppletWindow::load_user_info));
    }
    else
        load_user_info();

    natural_height = std::min(natural_height, rect.get_height());
    resize(natural_width, natural_height);
    Glib::signal_idle().connect_once(sigc::mem_fun(*this, &MenuAppletWindow::reload_apps_data));
}


MenuAppItem *MenuAppletWindow::create_app_item(std::shared_ptr<Kiran::App> app, Gtk::Orientation orient)
{

    auto item = Gtk::make_managed<MenuAppItem>(app);

    item->set_orientation(orient);
    item->signal_launched().connect(sigc::mem_fun(*this, &Gtk::Widget::hide));

    return item;
}

MenuCategoryItem *MenuAppletWindow::create_category_item(const std::string &name,
                                                             bool clickable)
{
    auto item = Gtk::make_managed<MenuCategoryItem>(name, clickable);

    return item;
}

void MenuAppletWindow::load_date_info()
{
    Gtk::Label *date_label;
    auto now = Glib::DateTime::create_now_local();

    builder->get_widget<Gtk::Label>("date-label", date_label);

    date_label->set_text(now.format(_("%a %b %e %Y")));
}
