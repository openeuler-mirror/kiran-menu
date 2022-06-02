#ifndef KIRANUSERINFO_H
#define KIRANUSERINFO_H

#include <sigc++/sigc++.h>
#include <sys/types.h>
#include <act/act.h>

class KiranUserInfo : public sigc::trackable
{
    using ready_signal_t = sigc::signal<void>;

public:
    KiranUserInfo(uid_t id);
    bool is_ready();
    const char *get_username();
    const char *get_iconfile();

    ready_signal_t signal_data_ready();

    friend void on_user_loaded(KiranUserInfo *info);

private:
    ready_signal_t m_signal_data_ready;

    uid_t uid;
    ActUser *user;
    guint handler_id;

    ActUserManager *manager;
};

#endif // KIRANUSERINFO_H
