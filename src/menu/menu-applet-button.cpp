#include "menu-applet-button.h"
#include <glibmm/i18n.h>
#include "kiran-helper.h"

#define BUTTON_MARGIN 6

MenuAppletButton::MenuAppletButton(MatePanelApplet *panel_applet):
    KiranAppletButton(panel_applet)
{
    set_tooltip_text(_("Kiran Start Menu"));
    set_icon_from_resource("/kiran-menu/icon/logo");
    get_style_context()->add_class("menu-applet-button");
}

void MenuAppletButton::on_toggled()
{
    if (!window.get_visible()) {
        MatePanelAppletOrient orient;
        int root_x, root_y;
        int window_width, window_height;
        Gtk::Allocation button_allocation;

        window.show();
        window.get_size(window_width, window_height);

        button_allocation = get_allocation();

        //获取按钮的位置坐标
        get_window()->get_origin(root_x, root_y);
        orient = mate_panel_applet_get_orient(get_applet());
        switch (orient)
        {
        case MATE_PANEL_APPLET_ORIENT_UP:
            root_y -= window_height;
            break;
        case MATE_PANEL_APPLET_ORIENT_DOWN:
            root_y += button_allocation.get_height();
            break;
        case MATE_PANEL_APPLET_ORIENT_LEFT:
            root_x -= window_width;
            break;
        case MATE_PANEL_APPLET_ORIENT_RIGHT:
            root_x += button_allocation.get_width();
            break;
        }
        window.move(root_x, root_y);
    } else {
        window.hide();
    }
}
