#ifndef KIRANWINDOWCONTEXTMENU_H
#define KIRANWINDOWCONTEXTMENU_H

#include "kiran-opacity-menu.h"
#include "app-manager.h"

class KiranWindowContextMenu : public KiranOpacityMenu
{
public:
    KiranWindowContextMenu(const std::shared_ptr<Kiran::Window> &win_);
    void refresh();
protected:
    Gtk::Menu *create_workspace_menu(void);

private:
    std::weak_ptr<Kiran::Window> win;

    Glib::RefPtr<Gdk::Window> g_window;
    Gtk::RadioButtonGroup group;
};

#endif // KIRANWINDOWCONTEXTMENU_H
