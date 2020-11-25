#ifndef MENU_USER_INFO_H
#define MENU_USER_INFO_H

#include <sigc++/sigc++.h>
#include <sys/types.h>
#include <act/act.h>

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
  ActUser *user;

  guint handler_id;
};

#endif // MENU_USER_INFO_H
