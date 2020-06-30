#include "kiran-menu-window.h"
#include "kiran-menu-app-launcher.h"
#include "kiran-menu-power-button.h"
#include "kiran-search-entry.h"
#include "kiranhelper.h"


#include <unistd.h>
#include <iostream>
#include <glibmm/i18n.h>

#define NEW_APPS_MAX_SIZE 3

KiranMenuWindow::KiranMenuWindow(Gtk::WindowType window_type):
    Gtk::Window(window_type)
{
    Gtk::Box *search_box;
    Glib::RefPtr<Gtk::StyleContext> context = get_style_context();

    set_name("menu-window");
    set_skip_taskbar_hint(true);
    set_skip_pager_hint(true);
    set_keep_above(true);
    set_decorated(false);
    set_accept_focus(true);
    set_focus_on_map(true);

    Kiran::MenuSkeleton::global_init();
    backend = Kiran::MenuSkeleton::get_instance();
    backend->signal_app_changed().connect(sigc::mem_fun(*this, &KiranMenuWindow::reload_apps_data));
    backend->signal_favorite_app_added().connect(sigc::hide(sigc::mem_fun(*this, &KiranMenuWindow::load_favorite_apps)));
    backend->signal_favorite_app_deleted().connect(sigc::hide(sigc::mem_fun(*this, &KiranMenuWindow::load_favorite_apps)));
    backend->signal_frequent_usage_app_changed().connect(sigc::mem_fun(*this, &KiranMenuWindow::load_frequent_apps));

    builder = Gtk::Builder::create_from_resource("/kiran-menu/ui/menu");
    builder->get_widget<Gtk::Box>("menu-container", box);
    builder->get_widget<Gtk::Grid>("sidebar-box", side_box);
    builder->get_widget<Gtk::Stack>("overview-stack", overview_stack);
    builder->get_widget<Gtk::Stack>("apps-view-stack", appview_stack);

    builder->get_widget<Gtk::Box>("new-apps-box", new_apps_box);
    builder->get_widget<Gtk::Box>("all-apps-box", all_apps_box);
    builder->get_widget<Gtk::Grid>("category-overview-box", category_overview_box);
    builder->get_widget<Gtk::Grid>("search-results-box", search_results_box);

    category_overview_box->set_orientation(Gtk::ORIENTATION_VERTICAL);

    profile.signal_changed().connect(sigc::mem_fun(*this, &Gtk::Widget::queue_draw));

    Gtk::EventBox *userinfo_box, *date_box;
    builder->get_widget<Gtk::EventBox>("userinfo-box", userinfo_box);
    builder->get_widget<Gtk::EventBox>("date-box", date_box);
    userinfo_box->add_events(Gdk::BUTTON_PRESS_MASK);
    userinfo_box->signal_button_press_event().connect([this](GdkEventButton *button) -> bool {
        std::vector<std::string> args;

        args.push_back("/usr/bin/mate-about-me");
        args.push_back("");

        this->hide();
        Glib::spawn_async(std::string(), args, Glib::SPAWN_STDOUT_TO_DEV_NULL | Glib::SPAWN_STDERR_TO_DEV_NULL | Glib::SPAWN_CLOEXEC_PIPES);

        return false;
    });

    date_box->add_events(Gdk::BUTTON_PRESS_MASK | Gdk::ENTER_NOTIFY_MASK | Gdk::LEAVE_NOTIFY_MASK);
    date_box->signal_button_press_event().connect([this](GdkEventButton *button) -> bool {
        std::vector<std::string> args;

        args.push_back("/usr/bin/system-config-date");
        args.push_back("");

        this->hide();
        Glib::spawn_async(std::string(), args, Glib::SPAWN_STDOUT_TO_DEV_NULL | Glib::SPAWN_STDERR_TO_DEV_NULL | Glib::SPAWN_CLOEXEC_PIPES);

        return false;
    });

    //添加搜索框
    builder->get_widget<Gtk::Box>("search-box", search_box);
    search_entry = Gtk::manage(new KiranSearchEntry());
    search_entry->set_can_default(true);
    search_entry->set_can_focus(true);
    search_entry->set_activates_default(true);
    search_box->add(*search_entry);

    search_entry->signal_search_changed().connect(sigc::mem_fun(*this, &KiranMenuWindow::on_search_change));
    search_entry->signal_stop_search().connect(sigc::mem_fun(*this, &KiranMenuWindow::on_search_stop));

    //添加侧边栏应用快捷方式
    add_sidebar_buttons();

    //通过reparent，将布局添加到当前窗口
    box->reparent(*this);
    box->show_all();

    load_all_apps();
    load_favorite_apps();
    load_frequent_apps();

    //加载当前用户信息
    user_info = new KiranUserInfo(getuid());
    if (!user_info->is_ready()) {
        user_info->signal_data_ready().connect(sigc::mem_fun(*this, &KiranMenuWindow::load_user_info));
    }
    else
        load_user_info();

    load_date_info();
    property_is_active().signal_changed().connect(sigc::mem_fun(*this, &KiranMenuWindow::on_active_change));
}

KiranMenuWindow::~KiranMenuWindow()
{
    delete user_info;
}

void KiranMenuWindow::reload_apps_data()
{
    load_all_apps();
    load_favorite_apps();
    load_frequent_apps();
}

void KiranMenuWindow::on_realize()
{
    Glib::RefPtr<Gdk::Visual> rgba_visual;
    Gdk::Rectangle rect;
    auto screen = Gdk::Screen::get_default();
    auto monitor = screen->get_display()->get_primary_monitor();

    /*设置窗口的Visual为RGBA visual，确保窗口背景透明度可以正常绘制 */
    rgba_visual = screen->get_rgba_visual();
    gtk_widget_set_visual(GTK_WIDGET(this->gobj()), rgba_visual->gobj());

    monitor->get_workarea(rect);

    set_size_request(-1, rect.get_height()*2/3);
    Gtk::Window::on_realize();
}

void KiranMenuWindow::on_active_change()
{
    if (is_active())
        KiranHelper::grab_input(*this);
}

/*
bool KiranMenuWindow::on_draw(const Cairo::RefPtr<Cairo::Context> &cr)
{
    double opacity;
    Gdk::RGBA background_color;
    Gtk::Widget *child;
    Gtk::Allocation allocation;
    auto context = get_style_context();

    allocation = get_allocation();
    background_color = profile.get_background_color();
    opacity = profile.get_opacity();
    cr->save();
    if (is_composited()) {
        //只有在composite启用的情况下，透明度才能绘制成功

        //TODO, 从gsettings中读取透明度设置
        background_color.set_alpha(opacity);
        Gdk::Cairo::set_source_rgba(cr, background_color);
        cr->set_operator(Cairo::OPERATOR_SOURCE);
    } else {
        background_color.set_alpha(1.0);
        Gdk::Cairo::set_source_rgba(cr, background_color);
    }
    cr->paint();
    cr->restore();

    context->render_frame(cr, 0, 0, allocation.get_width(), allocation.get_height());

    child = get_child();
    propagate_draw(*child, cr);
    return true;
}*/

/**
 * @brief 回调函数: 当搜索框内容变化时调用
 */
void KiranMenuWindow::on_search_change()
{
    if (search_entry->get_text_length() == 0) {
        //搜索内容为空，返回到应用列表页面
        on_search_stop();
        return;
    }

    //清空之前的搜索结果
    KiranHelper::remove_all_for_container(*search_results_box);

    //切换到搜索结果页面
    appview_stack->set_transition_type(Gtk::STACK_TRANSITION_TYPE_NONE);
    appview_stack->set_visible_child("search-results-page");

    //搜索时忽略关键字大小写
    auto apps_list = backend->search_app(search_entry->get_text().data(), true);
    if (apps_list.size()) {
        g_message("search results length %lu\n", apps_list.size());
        auto category_item = Gtk::manage(new KiranMenuCategoryItem(_("Search Results"), false));

        search_results_box->add(*category_item);
        for (auto iter = apps_list.begin(); iter != apps_list.end(); iter++) {
            auto app_item = create_app_item(*iter);

            search_results_box->add(*app_item);
        }
    } else {
        //搜索结果为空
        Gtk::Label *label = Gtk::manage(new Gtk::Label(_("No matched apps found!")));

        label->get_style_context()->add_class("search-empty-prompt");
        label->set_hexpand(true);
        label->set_vexpand(true);
        label->set_halign(Gtk::ALIGN_CENTER);
        label->set_valign(Gtk::ALIGN_CENTER);

        search_results_box->add(*label);
    }

    search_results_box->show_all();
}

/**
 * @brief 回调函数： 当搜索框中按下ESC键时调用。该函数只是返回到应用列表页面
 */
void KiranMenuWindow::on_search_stop()
{
    //返回应用列表页面
    appview_stack->set_visible_child("all-apps-page", Gtk::STACK_TRANSITION_TYPE_NONE);
}

/**
 * @brief 跳转到分类选择页面，并将跳转之前选择的分类标签对应的分类设置为已选择状态
 * @param button: 跳转前的分类选择标签，通常是被点击的那个
 */
void KiranMenuWindow::switch_to_category_overview(const std::string &selected_category)
{
    KiranMenuCategoryItem *selected_item = NULL;

    KiranHelper::remove_all_for_container(*category_overview_box);


    for (auto iter = category_names.begin(); iter != category_names.end(); iter++) {
        auto item = Gtk::manage(new KiranMenuCategoryItem(*iter, true));
        item->set_hexpand(true);
        item->show_all();

        if (item->get_category_name() == selected_category) {
            std::cout<<"found target category: "<<item->get_category_name()<<std::endl;
            selected_item = item;
        }
        item->signal_clicked().connect(sigc::bind<const std::string&>(
                                           sigc::mem_fun<const std::string&>(*this, &KiranMenuWindow::switch_to_apps_overview),
                                           item->get_category_name()));
        category_overview_box->add(*item);
    }

    if (selected_item)
        selected_item->grab_focus();
    overview_stack->set_transition_type(Gtk::STACK_TRANSITION_TYPE_CROSSFADE);
    overview_stack->set_visible_child("category-overview-page");
}

/**
 * @brief 跳转到所有应用页面，如果指定了要选择的分类名称，则将所有应用页面滚动到
 *        该分类对应的分类标签处(该分类标签位于滚动窗口内部的最上方位置）
 * @param selected_category：要选择的分类名称
 */
void KiranMenuWindow::switch_to_apps_overview(const std::string &selected_category)
{
    KiranMenuCategoryItem *item;
    Gtk::Allocation allocation;
    double adjusted_pos = -1;


    if (!selected_category.empty()) {

        //找到分类标签对应的控件
        auto iter = category_items.find(selected_category);
        if (iter != category_items.end()) {
            std::pair<std::string, KiranMenuCategoryItem*> data = *iter;
            item = data.second;

            //滚动到指定的分类标签
            allocation = item->get_allocation();
            adjusted_pos = allocation.get_y();
            switch_to_apps_overview(adjusted_pos);

            //将指定的分类标签控件添加焦点
            item->grab_focus();
        } else
            std::cout<<"invalid category name '"<<selected_category<<"'"<<std::endl;
    }
}

/**
 * @brief 切换到应用视图，并滚动到position对应的位置
 * @param position 要滚动到的位置，该位置将传递给应用列表所在viewport的adjustment
 */
void KiranMenuWindow::switch_to_apps_overview(double position)
{
    Gtk::Viewport *all_apps_viewport;

    if (position >= 0) {
        builder->get_widget<Gtk::Viewport>("all-apps-viewport", all_apps_viewport);

        auto adjustment = all_apps_viewport->get_vadjustment();
        adjustment->set_value(position);
    }

    //切换到应用程序列表
    overview_stack->set_visible_child("apps-overview-page", Gtk::STACK_TRANSITION_TYPE_CROSSFADE);
}

bool KiranMenuWindow::on_map_event(GdkEventAny *any_event)
{
    Gtk::Window::on_map_event(any_event);

    /**
     * 获取当前系统的鼠标事件，这样才能在鼠标点击窗口外部时及时隐藏窗口
     */
    KiranHelper::grab_input(*this);
    switch_to_apps_overview(0);
    on_search_stop();
    search_entry->set_text("");
    search_entry->grab_focus();

    g_message("window %d x %d\n", get_width(), get_height());
    return true;
}

bool KiranMenuWindow::on_unmap_event(GdkEventAny *any_event)
{
    KiranHelper::ungrab_input(*this);
    return Gtk::Window::on_unmap_event(any_event);
}

bool KiranMenuWindow::on_button_press_event(GdkEventButton *event)
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

bool KiranMenuWindow::on_key_press_event(GdkEventKey *key_event)
{
    if (key_event->keyval == GDK_KEY_Escape) {
        if (overview_stack->get_visible_child_name() == "category-overview-page") {
            //当前处于分类选择视图, 返回应用视图
            overview_stack->set_visible_child("apps-overview-page", Gtk::STACK_TRANSITION_TYPE_NONE);
            return true;
        }

        if (appview_stack->get_visible_child_name() == "search-results-page") {
            //当前处于搜索结果页面，需要返回应用列表页面
            appview_stack->set_visible_child("all-apps-page", Gtk::STACK_TRANSITION_TYPE_NONE);
            return true;
        } else {
            //隐藏开始菜单窗口
            hide();
            return true;
        }
    }

    //传递给子控件处理
    return Gtk::Window::on_key_press_event(key_event);
}

bool KiranMenuWindow::on_leave_notify_event(GdkEventCrossing *crossing_event)
{
    return false;
}

/**
 * @brief 添加快捷应用按钮到开始菜单的左侧侧边栏
 * @param icon_resource: 按钮图标的资源路径
 * @param tooltip: 按钮的工具提示文本
 * @param cmdline: 按钮点击时启动应用时调用的命令行参数，如 "/bin/ls /home"
 */
void KiranMenuWindow::add_app_button(const char *icon_resource,
                                     const char *tooltip,
                                     const char *cmdline)
{
    KiranMenuAppLauncher *button = Gtk::manage(new KiranMenuAppLauncher(icon_resource, tooltip, cmdline));

    button->signal_app_launched().connect(sigc::mem_fun(*this, &Gtk::Widget::hide));
    side_box->add(*button);
}

void KiranMenuWindow::add_sidebar_buttons()
{
    Gtk::Separator *separator;
    Gtk::Button *power_btn;

    separator= Gtk::manage(new Gtk::Separator(Gtk::ORIENTATION_HORIZONTAL));
    separator->set_margin_start(9);
    separator->set_margin_end(9);
    separator->set_margin_top(5);
    separator->set_margin_bottom(5);

    power_btn = Gtk::manage(new KiranMenuPowerButton());

    side_box->set_orientation(Gtk::ORIENTATION_VERTICAL);
    add_app_button("/kiran-menu/sidebar/home-dir", _("Home Directory"), "caja");
    add_app_button("/kiran-menu/sidebar/monitor", _("System monitor"), "mate-system-monitor");
    add_app_button("/kiran-menu/sidebar/help", _("Open help"), "yelp");
    side_box->add(*separator);
    add_app_button("/kiran-menu/sidebar/settings", _("Control center"), "mate-control-center");
    add_app_button("/kiran-menu/sidebar/lock", _("Lock Screen"), "mate-screensaver-command -l");
    side_box->add(*power_btn);

    side_box->show_all();
}

/**
 * @brief 加载系统应用列表
 */
void KiranMenuWindow::load_all_apps()
{
    Gtk::Box *new_apps_box, *all_apps_box;
    category_names = backend->get_category_names();

    builder->get_widget<Gtk::Box>("new-apps-box", new_apps_box);
    builder->get_widget<Gtk::Box>("all-apps-box", all_apps_box);

    //清空原有的应用和分类信息
    KiranHelper::remove_all_for_container(*new_apps_box);
    KiranHelper::remove_all_for_container(*all_apps_box);

    //加载新安装应用列表
    auto new_apps_list = backend->get_nnew_apps(-1);
    if (new_apps_list.size() > 0) {
        int index = 1;
        Gtk::Box *more_apps_box = nullptr;
        Gtk::ToggleButton *expand_button = nullptr;

        auto item = Gtk::manage(new KiranMenuCategoryItem(_("New Installed"), false));
        item->signal_focus_in_event().connect(sigc::bind<Gtk::Widget*>(
                                                  sigc::mem_fun(*this, &KiranMenuWindow::promise_item_viewable),
                                                  item));
        new_apps_box->add(*item);
        for (auto app: new_apps_list) {
            auto item = create_app_item(app);

            if (index <= NEW_APPS_MAX_SIZE)
                new_apps_box->add(*item);
            else {
                //新安装应用数量多，只显示部分应用和展开按钮
                if (!more_apps_box) {
                    auto image = Gtk::manage(new Gtk::Image());
                    image->set_from_resource("/kiran-menu/icon/expand");

                    expand_button = Gtk::manage(new Gtk::ToggleButton(_("Expand")));
                    more_apps_box = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL));
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

    //删除空的应用分类
    auto start = std::remove_if(category_names.begin(), category_names.end(),
		    [this](const std::string &name) -> bool {
		        return (this->backend->get_category_apps(name).size() == 0);
		    });
    category_names.erase(start, category_names.end());

    //遍历分类列表，建立应用列表
    for (auto category: category_names) {
        auto apps_list = backend->get_category_apps(category);
        auto item = Gtk::manage(new KiranMenuCategoryItem(category, true));

        item->signal_focus_in_event().connect(sigc::bind<Gtk::Widget*>(
                                                  sigc::mem_fun(*this, &KiranMenuWindow::promise_item_viewable),
                                                  item));
        category_items.insert(category_items.end(), std::pair<std::string, KiranMenuCategoryItem*>(category, item));
        item->signal_clicked().connect(sigc::bind<const std::string&>(
                                           sigc::mem_fun(*this, &KiranMenuWindow::switch_to_category_overview),
                                           item->get_category_name()));
        all_apps_box->add(*item);
        for (auto app: apps_list) {
            auto item = create_app_item(app);
            all_apps_box->add(*item);
        }
    }

    all_apps_box->show_all();
}

/**
 * @brief 加载常用应用信息
 */
void KiranMenuWindow::load_frequent_apps()
{
    KiranMenuListItem *item;
    Gtk::Box *frequent_apps_box, *frequent_header_box, *frequent_container;

    builder->get_widget<Gtk::Box>("frequent-apps-box", frequent_apps_box);
    builder->get_widget<Gtk::Box>("frequent-header-box", frequent_header_box);
    builder->get_widget<Gtk::Box>("frequent-box", frequent_container);
    frequent_apps_box->set_orientation(Gtk::ORIENTATION_HORIZONTAL);

    KiranHelper::remove_all_for_container(*frequent_apps_box);
    KiranHelper::remove_all_for_container(*frequent_header_box);

    auto apps_list = backend->get_nfrequent_apps(4);
    //apps_list.clear();
    if (apps_list.size() > 0) {
        item = Gtk::manage(new KiranMenuCategoryItem("Frequently used", false));
        frequent_header_box->add(*item);
        for (auto iter = apps_list.begin(); iter != apps_list.end(); iter++) {
            item = create_app_item(*iter, Gtk::ORIENTATION_VERTICAL);
            frequent_apps_box->pack_start(*item, Gtk::PACK_SHRINK);
        }
        frequent_container->show_all();
    } else {
        frequent_container->hide();
    }
}

/**
 * @brief 加载收藏夹应用列表
 */
void KiranMenuWindow::load_favorite_apps()
{
#define APP_COLUMN_COUNT 4                  //每行显示的app个数
    KiranMenuListItem *item;
    Gtk::Box *favorite_header_box;
    Gtk::Grid *favorite_apps_box;
    int index = 0;

    builder->get_widget<Gtk::Grid>("favorite-apps-box", favorite_apps_box);
    builder->get_widget<Gtk::Box>("favorite-header-box", favorite_header_box);

    KiranHelper::remove_all_for_container(*favorite_apps_box);
    KiranHelper::remove_all_for_container(*favorite_header_box);

    item = Gtk::manage(new KiranMenuCategoryItem("Favorite Apps", false));
    favorite_header_box->add(*item);

    auto apps_list = backend->get_favorite_apps();
    for (auto iter = apps_list.begin(); iter != apps_list.end(); iter++, index++) {
        item = create_app_item(*iter, Gtk::ORIENTATION_VERTICAL);
        favorite_apps_box->attach(*item, index % APP_COLUMN_COUNT, index / APP_COLUMN_COUNT, 1, 1);
    }

    favorite_header_box->show_all();
    favorite_apps_box->show_all();
}

/**
 * @brief 加载当前用户信息和头像
 */
void KiranMenuWindow::load_user_info()
{
    Gtk::Image *user_icon;
    Gtk::Label *name_label;
    Glib::RefPtr<Gdk::Pixbuf> avatar_pixbuf;

    if (!user_info->is_ready())
        return;

    builder->get_widget<Gtk::Label>("username-label", name_label);
    builder->get_widget<Gtk::Image>("avatar-icon", user_icon);

    name_label->set_markup(Glib::ustring(_("Hello")) + ", <b>" + user_info->get_username() +"</b>");
    try {
        avatar_pixbuf = Gdk::Pixbuf::create_from_file(user_info->get_iconfile(), 60, 60);
    } catch (const Glib::Error &e) {
	std::cerr<<"Failed to load user avatar from file: "<<e.what()<<std::endl;
	auto pixbuf = Gdk::Pixbuf::create_from_resource("/kiran-menu/icon/avatar");

        avatar_pixbuf = pixbuf->scale_simple(60, 60, Gdk::INTERP_BILINEAR);
    }
    if (avatar_pixbuf)
            user_icon->set(avatar_pixbuf);
}

bool KiranMenuWindow::promise_item_viewable(GdkEventFocus *event, Gtk::Widget *item)
{
    Gtk::Viewport *viewport = NULL;
    for (auto widget = item->get_parent(); widget != nullptr; widget = widget->get_parent()) {
        if (GTK_IS_VIEWPORT(widget->gobj())) {
                viewport = static_cast<Gtk::Viewport*>(widget);
                break;
        }
    }

    if (viewport) {
        Gtk::Allocation allocation;
        int view_height;
        double delta;
        double adjust_value;

        allocation = item->get_allocation();
        view_height  = viewport->get_view_window()->get_height();
        adjust_value = viewport->get_vadjustment()->get_value();

        delta = (allocation.get_y() + allocation.get_height()) - (adjust_value + view_height);
        if (delta > 0) {
            //控件绘制区域在滚动区域内并非全部可见，需要滚动
            viewport->get_vadjustment()->set_value(adjust_value + delta);
        } else if (allocation.get_y() < adjust_value) {
            viewport->get_vadjustment()->set_value((double)allocation.get_y());
        }
    }

    return false;
}

KiranMenuAppItem *KiranMenuWindow::create_app_item(std::shared_ptr<Kiran::App> app, Gtk::Orientation orient)
{

    auto item = Gtk::manage(new KiranMenuAppItem(app));

    item->set_orientation(orient);
    item->signal_launched().connect(sigc::mem_fun(*this, &Gtk::Widget::hide));
    item->signal_focus_in_event().connect(sigc::bind<Gtk::Widget*>(
                                              sigc::mem_fun(*this, &KiranMenuWindow::promise_item_viewable),
                                              item));


    return item;
}

void KiranMenuWindow::load_date_info()
{
    Gtk::Label *date_label;
    auto now = Glib::DateTime::create_now_local();

    builder->get_widget<Gtk::Label>("date-label", date_label);

    date_label->set_text(now.format(_("%a %b %e %Y")));
}
