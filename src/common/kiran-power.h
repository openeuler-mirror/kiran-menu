#ifndef KIRAN_POWER_INCLUDE_H
#define KIRAN_POWER_INCLUDE_H

#include <giomm.h>

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
bool lock_screen();

bool can_suspend();
bool can_hibernate();
bool can_switchuser();
bool can_reboot();

}

#endif //KIRAN_POWER_INCLUDE_H
