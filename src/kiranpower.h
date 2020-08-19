#ifndef KIRAN_POWER_INCLUDE_H
#define KIRAN_POWER_INCLUDE_H

#include <gtkmm.h>

#define LOGOUT_MODE_INTERACTIVE 0
#define LOGOUT_MODE_NOW 1

namespace KiranPower
{

bool suspend();
bool hibernate();
bool shutdown();
bool reboot();
bool logout(int mode);
bool switch_user();

bool can_suspend();
bool can_hibernate();
bool can_switchuser();

}

#endif //KIRAN_POWER_INCLUDE_H
