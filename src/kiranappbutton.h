#ifndef KIRANAPPBUTTON_H
#define KIRANAPPBUTTON_H

#include <gtkmm.h>

class KiranAppButton : public Gtk::Button
{
public:
    explicit KiranAppButton(const char *icon_file, const char *tooltip, const char *cmdline);

    sigc::signal<void> signal_app_launched();

protected:
    virtual void on_clicked() override;
    sigc::signal<void> m_signal_app_launched;

private:
    Gtk::Image icon;
    Glib::RefPtr<Gio::AppInfo> app;
};


#endif // KIRANAPPBUTTON_H
