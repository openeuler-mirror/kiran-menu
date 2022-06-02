#ifndef KIRAN_POWER_INCLUDE_H
#define KIRAN_POWER_INCLUDE_H

#include <gtkmm.h>

namespace KiranPower
{

bool suspend();
bool hibernate();
bool shutdown();
bool reboot();
bool logout();

bool can_suspend();
bool can_hibernate();
bool can_reboot();

}

#endif //KIRAN_POWER_INCLUDE_H
