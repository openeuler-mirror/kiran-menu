#ifndef __MENU_NEW_APPS_CONTAINER_H__
#define __MENU_NEW_APPS_CONTAINER_H__

#include "menu-apps-container.h"

class MenuNewAppsContainer : public MenuAppsContainer
{
public:
    MenuNewAppsContainer(int max_apps_);
    bool load_applications(const Kiran::AppVec &apps);

protected:
    Gtk::ToggleButton *create_expand_button();

private:
    int max_apps;
    Gtk::Box *more_apps_box;
    Gtk::ToggleButton *expand_button;
    std::vector<MenuAppItem*> hidden_items;
};

#endif // __MENU_NEW_APPS_CONTAINER_H__