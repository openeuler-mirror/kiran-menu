#include "kiran-menu-applet-button.h"
#include <glibmm/i18n.h>

#define BUTTON_MARGIN 6
#define UNUSED __attribute__((unused))

KiranMenuAppletButton::KiranMenuAppletButton(MatePanelApplet *panel_applet):
    KiranAppletButton(panel_applet)
{
    set_tooltip_text(_("Kiran Start Menu"));
    set_icon_from_resource("/kiran-menu/icon/logo");

    /**
     * 当窗口隐藏时更新插件按钮状态
    */
    connection1 = window.signal_hide().connect(sigc::bind<bool>(sigc::mem_fun(*this, &Gtk::ToggleButton::set_active), false));
    window.signal_size_changed().connect(
                [this](int width UNUSED, int height UNUSED) -> void {
                    this->on_toggled();
                });

    get_style_context()->add_class("kiran-menu-applet-button");
}

KiranMenuAppletButton::~KiranMenuAppletButton()
{
    connection1.disconnect();
}

void KiranMenuAppletButton::on_toggled()
{
    if (get_active()) {
        MatePanelAppletOrient orient;
        int root_x, root_y;
        Gtk::Allocation button_allocation, window_allocation;

        window.show();

        button_allocation = get_allocation();
        window_allocation = window.get_allocation();

        //获取按钮的位置坐标
        get_window()->get_origin(root_x, root_y);
        orient = mate_panel_applet_get_orient(get_applet());
        switch (orient)
        {
        case MATE_PANEL_APPLET_ORIENT_UP:
            root_y -= window_allocation.get_height();
            break;
        case MATE_PANEL_APPLET_ORIENT_DOWN:
            root_y += button_allocation.get_height();
            break;
        case MATE_PANEL_APPLET_ORIENT_LEFT:
            root_x -= window_allocation.get_width();
            break;
        case MATE_PANEL_APPLET_ORIENT_RIGHT:
            root_x += button_allocation.get_width();
            break;
        }

        window.move(root_x, root_y);
    } else
        window.hide();
}
