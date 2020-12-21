#ifndef MENU_USER_INFO_H
#define MENU_USER_INFO_H

#include <sigc++/sigc++.h>
#include <sys/types.h>
#include "../config.h"

#ifdef BUILD_WITH_KIRANACCOUNTS
#include "kiran-accounts-manager.h"
#else
#include <act/act.h>
#endif


class MenuUserInfo : public sigc::trackable
{
public:
  MenuUserInfo(uid_t id);
  ~MenuUserInfo();

  bool is_ready() const;
  bool load();

  const char *get_username() const;
  const char *get_iconfile() const;

  sigc::signal<void> signal_ready();
  sigc::signal<void> signal_changed();

  static void on_loaded(MenuUserInfo *info);
  static void on_changed(MenuUserInfo *info);  

private:
  sigc::signal<void> m_signal_ready;
  sigc::signal<void> m_signal_changed;

  uid_t uid;
#ifdef BUILD_WITH_ACCOUNTSSERVICE
  ActUser *user;
#else
  KiranAccountsUser *user;
#endif

  guint handler_id;
  int load_state;
};

#endif // MENU_USER_INFO_H
