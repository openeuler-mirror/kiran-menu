#ifndef KIRANAPPCONTEXTMENU_H
#define KIRANAPPCONTEXTMENU_H

#include "kiran-opacity-menu.h"
#include "app.h"

class KiranAppContextMenu : public KiranOpacityMenu
{
public:
    KiranAppContextMenu(const std::shared_ptr<Kiran::App> &app);

    void refresh();

private:
    std::weak_ptr<Kiran::App> app;
};

#endif // KIRANAPPCONTEXTMENU_H
