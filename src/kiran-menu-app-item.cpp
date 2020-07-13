#include "kiran-menu-app-item.h"
#include "kiranhelper.h"
#include <iostream>
#include <glibmm/i18n.h>
#include <sys/stat.h>

#define MENU_ITEM_COUNT G_N_ELEMENTS(item_labels)

KiranMenuAppItem::KiranMenuAppItem(std::shared_ptr<Kiran::App> _app, int _icon_size, Gtk::Orientation orient):
    Glib::ObjectBase("KiranMenuAppItem"),
    KiranMenuListItem(_icon_size, orient),
    app(_app)
{
    auto context = get_style_context();

    context->add_class("kiran-app-item");

    set_text(app->get_locale_name());

    if (app->get_icon()) {
        set_icon(app->get_icon());
    } else {
        g_message("app '%s' has no icon\n", app->get_file_name().data());
        auto icon = Gio::ThemedIcon::create("application-x-executable");
        set_icon(Glib::RefPtr<Gio::Icon>::cast_dynamic(icon));
    }

    set_tooltip_text(app->get_locale_comment());
}

KiranMenuAppItem::~KiranMenuAppItem() {
}

bool KiranMenuAppItem::on_button_press_event(GdkEventButton *button_event)
{
    if (gdk_event_triggers_context_menu((GdkEvent*)button_event)) {
        //鼠标右键点击，显示上下文菜单
        create_context_menu();
        context_menu.popup_at_pointer((GdkEvent*)button_event);
        return false;
    }

    //鼠标左键点击启动app
    launch_app();
    return false;
}

bool KiranMenuAppItem::on_key_press_event(GdkEventKey *key_event)
{
    switch(key_event->keyval) {
    case GDK_KEY_Menu:
        do {
            Gtk::Allocation allocation;

            allocation = get_allocation();
            allocation.set_x(0);
            allocation.set_y(0);

            //重新创建右键菜单，以确保收藏夹相关的选项能及时更新
            create_context_menu();
            context_menu.popup_at_rect(get_window(), allocation,
                                       Gdk::GRAVITY_CENTER,
                                       Gdk::GRAVITY_NORTH_WEST,
                                       (GdkEvent*)key_event);
        } while (0);
        break;
    case GDK_KEY_Return:
        launch_app();
        break;
    default:
        break;
    }

    return false;
}

void KiranMenuAppItem::create_context_menu()
{
    Gtk::MenuItem *item;
    auto backend = Kiran::MenuSkeleton::get_instance();

    KiranHelper::remove_all_for_container(context_menu);

    item = Gtk::make_managed<Gtk::MenuItem>(_("Launch"));
    item->signal_activate().connect(sigc::hide_return(sigc::mem_fun(*this, &KiranMenuAppItem::launch_app)));
    context_menu.append(*item);

    item = Gtk::make_managed<Gtk::MenuItem>(_("Add to desktop"));
    item->signal_activate().connect([item, this]() {
        std::string target_dir;
        Gio::FileCopyFlags flags = Gio::FILE_COPY_BACKUP | Gio::FILE_COPY_TARGET_DEFAULT_PERMS;

        target_dir = Glib::get_user_special_dir(Glib::USER_DIRECTORY_DESKTOP);
        try {
            auto src_file = Gio::File::create_for_path(this->app->get_file_name());
            auto dest_file = Gio::File::create_for_path(target_dir + "/" + src_file->get_basename());

            //如果桌面上已经存在相同的文件，跳过拷贝操作
            if (dest_file->query_exists())
                return;

            if (!src_file->copy(dest_file, flags))
                std::cerr<<"Failed to copy"<<std::endl;

            //将desktop文件标记为可执行
            chmod(dest_file->get_path().data(), 0755);
        } catch (const Glib::Error &e) {
            std::cerr<<"Error occured while trying to copy desktop file: "<<e.what()<<std::endl;
        }
    });
    context_menu.append(*item);

    if (!is_in_favorite()) {
        item = Gtk::make_managed<Gtk::MenuItem>(_("Add to favorites"));
        item->signal_activate().connect(sigc::hide_return(sigc::bind<const std::string&>(sigc::mem_fun(*backend, &Kiran::MenuSkeleton::add_favorite_app), app->get_desktop_id())));
    } else {
        item = Gtk::make_managed<Gtk::MenuItem>(_("Remove from favorites"));
        item->signal_activate().connect(sigc::hide_return(
                                            sigc::bind<const std::string&>(
                                                sigc::mem_fun(*backend, &Kiran::MenuSkeleton::del_favorite_app), app->get_desktop_id()
                                            )));
    }

    context_menu.append(*item);

    if (!context_menu.get_attach_widget())
        context_menu.attach_to_widget(*this);
    context_menu.show_all();
}

bool KiranMenuAppItem::is_in_favorite()
{
    auto backend = Kiran::MenuSkeleton::get_instance();

    if (backend->lookup_favorite_app(app->get_desktop_id()) == nullptr)
        return false;
    return true;
}

sigc::signal<void> KiranMenuAppItem::signal_launched()
{
    return m_signal_launched;
}

void KiranMenuAppItem::set_orientation(Gtk::Orientation orient)
{
    auto context = get_style_context();

    if (orient == Gtk::ORIENTATION_VERTICAL) {
        set_icon_size(56);
        context->remove_class("kiran-app-item");
        context->add_class("kiran-app-entry");
    } else {
        set_icon_size(24);
        context->remove_class("kiran-app-entry");
        context->add_class("kiran-app-item");
    }

    KiranMenuListItem::set_orientation(orient);
}

void KiranMenuAppItem::launch_app()
{
    app->launch();
    signal_launched().emit();
}
