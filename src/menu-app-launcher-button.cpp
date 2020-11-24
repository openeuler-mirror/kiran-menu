#include "menu-app-launcher-button.h"

MenuAppLauncherButton::MenuAppLauncherButton(const char *icon_file, const char *tooltip, const char *cmdline)
{
    auto context = get_style_context();
    set_tooltip_text(tooltip);

    icon.set_from_resource(icon_file);
    add(icon);
    set_hexpand(true);
    set_halign(Gtk::ALIGN_FILL);

    app = Gio::AppInfo::create_from_commandline(cmdline, std::string(),
                 Gio::APP_INFO_CREATE_SUPPORTS_STARTUP_NOTIFICATION);

    context->add_class("menu-app-launcher");
}

void MenuAppLauncherButton::on_clicked()
{
    std::vector<Glib::RefPtr<Gio::File>> files;

    files.clear();
    app->launch(files);

    signal_app_launched().emit();
}

sigc::signal<void> MenuAppLauncherButton::signal_app_launched()
{
    return m_signal_app_launched;
}
