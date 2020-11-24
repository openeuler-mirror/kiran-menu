#include "menu-new-apps-container.h"
#include "kiran-helper.h"
#include <glibmm/i18n.h>

bool MenuNewAppsContainer::load_applications(const Kiran::AppVec &apps)
{
    int index = 0;
    Gtk::FlowBox &apps_box = get_apps_box();

    KiranHelper::remove_all_for_container(apps_box);
    hidden_items.clear();

    apps_box.set_min_children_per_line(1);
    apps_box.set_max_children_per_line(1);
    for (auto app : apps)
    {
        auto child_box = Gtk::make_managed<Gtk::FlowBoxChild>();
        auto item = Gtk::manage(create_app_item(app, Gtk::ORIENTATION_HORIZONTAL));

        item->set_hexpand(true);
        item->set_halign(Gtk::ALIGN_FILL);

        child_box->set_can_focus(false);
        child_box->add(*item);
        if (index < max_apps)
        {
            apps_box.add(*child_box);
            child_box->show_all();
        }
        else
        {
            //新安装应用数量多，只显示部分应用和展开按钮
            if (!expand_button)
            {
                auto child_box = Gtk::make_managed<Gtk::FlowBoxChild>();
                expand_button = Gtk::manage(create_expand_button());

                child_box->set_can_focus(false);
                child_box->add(*expand_button);
                apps_box.add(*child_box);
                child_box->show_all();
            }

            apps_box.add(*child_box);
            child_box->hide();
            hidden_items.push_back(item);
        }

        index++;
    }

    apps_box.show();

    return true;
}

Gtk::ToggleButton *MenuNewAppsContainer::create_expand_button()
{
    auto button = new Gtk::ToggleButton(_("Expand"));
    auto image = new Gtk::Image();

    image->set_from_resource("/kiran-menu/icon/expand");
    image->set_margin_left(5);

    button->set_image(*image);
    button->set_image_position(Gtk::POS_RIGHT);
    button->set_alignment(0.0, 0.5);
    button->get_style_context()->add_class("menu-expand-button");

    button->signal_toggled().connect(
        [this]() -> void {
            auto image = dynamic_cast<Gtk::Image *>(expand_button->get_image());

            if (expand_button->get_active())
            {
                expand_button->set_label(_("Shrink"));
                image->set_from_resource("/kiran-menu/icon/shrink");
                for (auto app_item : hidden_items)
                {
                    app_item->get_parent()->show_all();
                }
            }
            else
            {
                expand_button->set_label(_("Expand"));
                image->set_from_resource("/kiran-menu/icon/expand");
                for (auto app_item : hidden_items)
                {
                    app_item->get_parent()->hide();
                }
            }
        });

    return button;
}

MenuNewAppsContainer::MenuNewAppsContainer(int max_apps_) : MenuAppsContainer(MenuAppsContainer::ICON_MODE_SMALL, _("New Installed"), false, false),
                                                            more_apps_box(nullptr),
                                                            expand_button(nullptr),
                                                            max_apps(max_apps_)
{
    set_auto_hide(true);
    set_draw_frame(false);
}
