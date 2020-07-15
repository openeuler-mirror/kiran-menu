#ifndef KIRANWINDOWCONTEXTMENU_H
#define KIRANWINDOWCONTEXTMENU_H

#include "kiran-opacity-menu.h"
#include "app-manager.h"

class KiranWindowContextMenu : public KiranOpacityMenu
{
public:
    KiranWindowContextMenu(const std::shared_ptr<Kiran::Window> &win_);
    void refresh();
private:
    std::weak_ptr<Kiran::Window> win;

    Glib::RefPtr<Gdk::Window> g_window;
};

#endif // KIRANWINDOWCONTEXTMENU_H
