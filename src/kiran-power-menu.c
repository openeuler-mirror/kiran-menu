#include "kiran-power-menu.h"
#include <glib/gi18n.h>

struct _KiranPowerMenu
{
    GtkPopover parent;

    GDBusProxy *session_manager_proxy, *login_manager_proxy;
    GtkWidget *grid;
};

struct _KiranPowerMenuClass
{
    GtkPopoverClass parent_class;

    void (*shutdown)(KiranPowerMenu *self);
    void (*reboot)(KiranPowerMenu *self);
    void (*suspend)(KiranPowerMenu *self);
    void (*hibernate)(KiranPowerMenu *self);
    void (*logout)(KiranPowerMenu *self);
};

G_DEFINE_TYPE(KiranPowerMenu, kiran_power_menu, GTK_TYPE_POPOVER)

#define SESSION_MANAGER_DBUS "org.gnome.SessionManager"
#define SESSION_MANAGER_PATH "/org/gnome/SessionManager"
#define SESSION_MANAGER_INTERFACE "org.gnome.SessionManager"

#define LOGIN_MANAGER_DBUS "org.freedesktop.login1"
#define LOGIN_MANAGER_PATH "/org/freedesktop/login1"
#define LOGIN_MANAGER_INTERFACE "org.freedesktop.login1.Manager"

#define LOGOUT_MODE_INTERACTIVE 0
#define LOGOUT_MODE_NOW 1

typedef struct {
    const char *label;
    guint callback_offset;
} PowerAction;

static PowerAction actions[] = {
    {"Logout", G_STRUCT_OFFSET(KiranPowerMenuClass, logout)},
    {"Shutdown", G_STRUCT_OFFSET(KiranPowerMenuClass, shutdown)},
    {"Reboot", G_STRUCT_OFFSET(KiranPowerMenuClass, reboot)},
    {"Suspend", G_STRUCT_OFFSET(KiranPowerMenuClass, suspend)},
    {"Hibernate", G_STRUCT_OFFSET(KiranPowerMenuClass, hibernate)}
};

void kiran_power_menu_init(KiranPowerMenu *self)
{
    GError *error = NULL;

    self->session_manager_proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SESSION,
                                                G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES | G_DBUS_PROXY_FLAGS_DO_NOT_CONNECT_SIGNALS,
                                                NULL,
                                                SESSION_MANAGER_DBUS,
                                                SESSION_MANAGER_PATH,
                                                SESSION_MANAGER_INTERFACE,
                                                NULL, &error);

    if (!self->session_manager_proxy)
    {
        g_critical("Failed to connect to session manager: '%s'\n", error->message);
        g_error_free(error);
    }

    self->login_manager_proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SESSION,
                                                G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES | G_DBUS_PROXY_FLAGS_DO_NOT_CONNECT_SIGNALS,
                                                NULL,
                                                LOGIN_MANAGER_DBUS,
                                                LOGIN_MANAGER_PATH,
                                                LOGIN_MANAGER_INTERFACE,
                                                NULL, &error);

    if (!self->login_manager_proxy)
    {
        g_critical("Failed to connect to login manager: '%s'\n", error->message);
        g_error_free(error);
    }

    self->grid = gtk_grid_new();
    for (int i = 0; i < G_N_ELEMENTS(actions); i++)
    {
        GtkWidget *button;
        gpointer *ptr, callback;

        KiranPowerMenuClass *kclass = G_TYPE_INSTANCE_GET_CLASS(self, KIRAN_TYPE_POWER_MENU, KiranPowerMenuClass);

        ptr = G_STRUCT_MEMBER_P(kclass, actions[i].callback_offset);
        callback = *ptr;

        button = gtk_button_new_with_label(_(actions[i].label));
        gtk_grid_attach(GTK_GRID(self->grid), button, 0, i, 1, 1);
        gtk_button_set_alignment(GTK_BUTTON(button), 0.0, 0.5);
        g_signal_connect_swapped(button, "clicked", G_CALLBACK(callback), self);
    }
    gtk_container_add(GTK_CONTAINER(self), self->grid);
    gtk_widget_show_all(self->grid);
}

void kiran_power_menu_finalize(GObject *obj)
{
    KiranPowerMenu *self = KIRAN_POWER_MENU(obj);

    if (self->session_manager_proxy)
        g_object_unref(self->session_manager_proxy);
    
    if (self->login_manager_proxy)
        g_object_unref(self->login_manager_proxy);

    G_OBJECT_CLASS(kiran_power_menu_parent_class)->finalize(obj);
}

void kiran_power_menu_shutdown(KiranPowerMenu *self)
{
    GError *error = NULL;

    g_assert(self->session_manager_proxy != NULL);

    g_dbus_proxy_call_sync(self->session_manager_proxy, "RequestShutdown", NULL, G_DBUS_CALL_FLAGS_NONE, 300, NULL, &error);

    if (error)
    {
        g_critical("Failed to call 'RequestShutdown' dbus method: '%s'\n", error->message);
        g_error_free(error);
        return;
    }
}

void kiran_power_menu_reboot(KiranPowerMenu *self)
{
    GError *error = NULL;

    g_assert(self->session_manager_proxy != NULL);

    g_dbus_proxy_call_sync(self->session_manager_proxy, "RequestReboot", NULL, G_DBUS_CALL_FLAGS_NONE, 300, NULL, &error);

    if (error)
    {
        g_critical("Failed to call 'RequestReboot' dbus method: '%s'\n", error->message);
        g_error_free(error);
        return;
    }
}

void kiran_power_menu_suspend(KiranPowerMenu *self)
{
    GError *error = NULL;

    g_assert(self->login_manager_proxy != NULL);
    g_dbus_proxy_call_sync(self->login_manager_proxy, "Suspend", g_variant_new_boolean(FALSE), G_DBUS_CALL_FLAGS_NONE, 300, NULL, &error);

    if (error)
    {
        g_critical("Failed to call 'Suspend' dbus method: '%s'\n", error->message);
        g_error_free(error);
        return;
    }
}

void kiran_power_menu_hibernate(KiranPowerMenu *self)
{
    GError *error = NULL;

    g_assert(self->login_manager_proxy != NULL);

    g_dbus_proxy_call_sync(self->login_manager_proxy, "Hibernate", g_variant_new_boolean(FALSE), G_DBUS_CALL_FLAGS_NONE, 300, NULL, &error);

    if (error)
    {
        g_critical("Failed to call 'Hibernate' dbus method: '%s'\n", error->message);
        g_error_free(error);
        return;
    }
}

void kiran_power_menu_logout(KiranPowerMenu *self)
{
    GError *error = NULL;

    g_assert(self->session_manager_proxy != NULL);

    g_dbus_proxy_call_sync(self->session_manager_proxy, "Logout",
                           g_variant_new_uint32(LOGOUT_MODE_INTERACTIVE),
                           G_DBUS_CALL_FLAGS_NONE,
                           300,
                           NULL,
                           &error);

    if (error)
    {
        g_critical("Failed to call 'Logout' dbus method: '%s'\n", error->message);
        g_error_free(error);
        return;
    }
}

void kiran_power_menu_class_init(KiranPowerMenuClass *kclass)
{
    kclass->logout = kiran_power_menu_logout;
    kclass->reboot = kiran_power_menu_reboot;
    kclass->shutdown = kiran_power_menu_shutdown;
    kclass->suspend = kiran_power_menu_suspend;
    kclass->hibernate = kiran_power_menu_hibernate;

    G_OBJECT_CLASS(kclass)->finalize = kiran_power_menu_finalize;
}

KiranPowerMenu *kiran_power_menu_new(void)
{
    return g_object_new(KIRAN_TYPE_POWER_MENU, "position", GTK_POS_RIGHT, NULL);
}
