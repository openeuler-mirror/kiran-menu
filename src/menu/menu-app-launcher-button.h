#ifndef MENU_APPLICATION_LAUNCHER_BUTTON_H
#define MENU_APPLICATION_LAUNCHER_BUTTON_H

#include <gtkmm.h>

class MenuAppLauncherButton : public Gtk::Button
{
public:
    explicit MenuAppLauncherButton(const char *icon_file, const char *tooltip, const char *cmdline);

    sigc::signal<void> signal_app_launched();

protected:
    virtual void on_clicked() override;
    sigc::signal<void> m_signal_app_launched;

private:
    Gtk::Image icon;
    Glib::RefPtr<Gio::AppInfo> app;
};


#endif // MENU_APPLICATION_LAUNCHER_BUTTON_H
