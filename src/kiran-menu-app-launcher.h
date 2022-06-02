#ifndef KIRAN_MENU_APP_LAUNCHER_H
#define KIRAN_MENU_APP_LAUNCHER_H

#include <gtkmm.h>

class KiranMenuAppLauncher : public Gtk::Button
{
public:
    explicit KiranMenuAppLauncher(const char *icon_file, const char *tooltip, const char *cmdline);

    sigc::signal<void> signal_app_launched();

protected:
    virtual void on_clicked() override;
    sigc::signal<void> m_signal_app_launched;

private:
    Gtk::Image icon;
    Glib::RefPtr<Gio::AppInfo> app;
};


#endif // KIRAN_MENU_APP_LAUNCHER_H
