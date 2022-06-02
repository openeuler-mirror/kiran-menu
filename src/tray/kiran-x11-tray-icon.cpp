/**
 * Copyright (c) 2020 ~ 2021 KylinSec Co., Ltd. 
 * kiran-cc-daemon is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2. 
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2 
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, 
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, 
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.  
 * See the Mulan PSL v2 for more details.  
 * 
 * Author:     wangxiaoqing <wangxiaoqing@kylinos.com.cn>
 */

#include <app-manager.h>
#include <glib/gstdio.h>

#include "kiran-notify-icon.h"
#include "kiran-x11-tray-icon.h"
#include "kiran-x11-tray-socket.h"
#include "lib/base.h"

struct _KiranX11TrayIconPrivate
{
    Window icon_window;
    gchar *id;
    gchar *name;
    KiranNotifyIconCategory category;
    gchar *icon;
    GtkWidget *socket;
    gboolean has_alpha;
    gchar *app_category;

    gboolean has_desktop; /* 是否找到对应窗口的桌面文件 */
};

enum
{
    PROP_0,
    PROP_ORIENTATION
};

static void kiran_notify_icon_init(KiranNotifyIconInterface *iface);
static const char *kiran_x11_tray_icon_get_id(KiranNotifyIcon *icon);
static const char *kiran_x11_tray_icon_get_name(KiranNotifyIcon *icon);
static KiranNotifyIconCategory kiran_x11_tray_icon_get_category(KiranNotifyIcon *icon);
static const char *kiran_x11_tray_icon_get_app_category(KiranNotifyIcon *icon);
static KiranNotifyIconWay kiran_x11_tray_icon_get_way(KiranNotifyIcon *icon);
static const char *kiran_x11_tray_icon_get_icon(KiranNotifyIcon *icon);

#define KIRAN_X11_TRAY_ICON_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), KIRAN_TYPE_X11_TRAY_ICON, KiranX11TrayIconPrivate))

G_DEFINE_TYPE_WITH_CODE(KiranX11TrayIcon, kiran_x11_tray_icon, GTK_TYPE_EVENT_BOX,
                        G_IMPLEMENT_INTERFACE(GTK_TYPE_ORIENTABLE, NULL)
                            G_IMPLEMENT_INTERFACE(KIRAN_TYPE_NOTIFY_ICON, kiran_notify_icon_init))

static void
kiran_notify_icon_init(KiranNotifyIconInterface *iface)
{
    iface->get_id = kiran_x11_tray_icon_get_id;
    iface->get_name = kiran_x11_tray_icon_get_name;
    iface->get_icon = kiran_x11_tray_icon_get_icon;
    iface->get_category = kiran_x11_tray_icon_get_category;
    iface->get_app_category = kiran_x11_tray_icon_get_app_category;
    iface->get_way = kiran_x11_tray_icon_get_way;
}

static GObject *
kiran_x11_tray_icon_constructor(GType type,
                                guint n_construct_properties,
                                GObjectConstructParam *construct_properties)
{
    GObject *obj;
    KiranX11TrayIcon *icon;

    obj = G_OBJECT_CLASS(kiran_x11_tray_icon_parent_class)->constructor(type, n_construct_properties, construct_properties);
    icon = KIRAN_X11_TRAY_ICON(obj);

    gtk_widget_set_can_focus(GTK_WIDGET(icon), TRUE);

    icon->priv->socket = kiran_x11_tray_socket_new();
    gtk_container_add(GTK_CONTAINER(icon), icon->priv->socket);
    gtk_widget_show(icon->priv->socket);

    return obj;
}

static void
kiran_x11_tray_icon_finalize(GObject *object)
{
    KiranX11TrayIconPrivate *priv = KIRAN_X11_TRAY_ICON(object)->priv;

    g_free(priv->id);
    g_free(priv->name);
    g_free(priv->icon);
    g_free(priv->app_category);
}

static void
kiran_x11_tray_icon_get_property(GObject *object,
                                 guint property_id,
                                 GValue *value,
                                 GParamSpec *pspec)
{
    switch (property_id)
    {
    case PROP_ORIENTATION:
        g_value_set_enum(value, GTK_ORIENTATION_HORIZONTAL);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void
kiran_x11_tray_icon_set_property(GObject *object,
                                 guint property_id,
                                 const GValue *value,
                                 GParamSpec *pspec)
{
    switch (property_id)
    {
    case PROP_ORIENTATION:
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static gboolean
kiran_x11_tray_icon_draw(GtkWidget *widget,
                         cairo_t *cr)
{
    KiranX11TrayIcon *icon = KIRAN_X11_TRAY_ICON(widget);
    KiranX11TrayIconPrivate *priv = icon->priv;

    GTK_WIDGET_CLASS(kiran_x11_tray_icon_parent_class)->draw(widget, cr);

    kiran_x11_tray_socket_draw_on_parent(KIRAN_X11_TRAY_SOCKET(priv->socket),
                                         widget,
                                         cr);
    if (!priv->has_desktop)
    {
        /* 如果没有对应的桌面文件， 则保存一张对应的窗口图片 */

        if (!g_file_test(priv->icon, G_FILE_TEST_EXISTS))
        {
            /* 还没有保存过， 则进行保存 */
            GdkWindow *gdk_window;
            cairo_t *cairo;
            cairo_surface_t *surface;

            gdk_window = gtk_widget_get_window(priv->socket);
            G_GNUC_BEGIN_IGNORE_DEPRECATIONS
            cairo = gdk_cairo_create(gdk_window);
            G_GNUC_END_IGNORE_DEPRECATIONS

            if (cairo)
            {
                surface = cairo_get_target(cairo);
                if (surface)
                {
                    gchar *dir = NULL;

                    dir = g_strdup_printf("%s/.config/kiran-tray/icon", g_get_home_dir());
                    if (dir == NULL)
                    {
                        cairo_destroy(cairo);
                        return TRUE;
                    }

                    if (!g_file_test(dir, G_FILE_TEST_EXISTS))
                    {
                        g_mkdir_with_parents(dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
                    }
                    free(dir);

                    cairo_surface_write_to_png(surface, priv->icon);
                }

                cairo_destroy(cairo);
            }
        }
    }

    return TRUE;
}

static gboolean
kiran_x11_tray_icon_leave_notify_event(GtkWidget *widget,
                                       GdkEventCrossing *event)
{
    GtkStateFlags flags;

    flags = gtk_widget_get_state_flags(widget);
    flags = (GtkStateFlags)(flags & ~GTK_STATE_FLAG_PRELIGHT);

    gtk_widget_set_state_flags(widget, flags, TRUE);

    return FALSE;
}

static gboolean
kiran_x11_tray_icon_enter_notify_event(GtkWidget *widget,
                                       GdkEventCrossing *event)
{
    gtk_widget_grab_focus(widget);

    gtk_widget_set_state_flags(widget, GTK_STATE_FLAG_PRELIGHT, FALSE);

    return FALSE;
}

static gboolean
kiran_x11_tray_icon_focus_out_event(GtkWidget *widget,
                                    GdkEventFocus *event)
{
    GtkStateFlags flags;

    flags = gtk_widget_get_state_flags(widget);
    flags = (GtkStateFlags)(flags & ~GTK_STATE_FLAG_PRELIGHT);
    gtk_widget_set_state_flags(widget, flags, TRUE);

    return FALSE;
}

static gboolean
kiran_x11_tray_icon_focus_in_event(GtkWidget *widget,
                                   GdkEventFocus *event)
{
    gtk_widget_set_state_flags(widget, GTK_STATE_FLAG_PRELIGHT, FALSE);

    return FALSE;
}

static void
kiran_x11_tray_icon_class_init(KiranX11TrayIconClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

    gobject_class->constructor = kiran_x11_tray_icon_constructor;
    gobject_class->finalize = kiran_x11_tray_icon_finalize;
    gobject_class->get_property = kiran_x11_tray_icon_get_property;
    gobject_class->set_property = kiran_x11_tray_icon_set_property;

    widget_class->leave_notify_event = kiran_x11_tray_icon_leave_notify_event;
    widget_class->enter_notify_event = kiran_x11_tray_icon_enter_notify_event;
    widget_class->focus_out_event = kiran_x11_tray_icon_focus_out_event;
    widget_class->focus_in_event = kiran_x11_tray_icon_focus_in_event;
    widget_class->draw = kiran_x11_tray_icon_draw;

    g_type_class_add_private(gobject_class, sizeof(KiranX11TrayIconPrivate));
    g_object_class_override_property(gobject_class, PROP_ORIENTATION, "orientation");
}

static void
kiran_x11_tray_icon_init(KiranX11TrayIcon *self)
{
    KiranX11TrayIconPrivate *priv;

    priv = self->priv = KIRAN_X11_TRAY_ICON_GET_PRIVATE(self);
    priv->id = NULL;
    priv->name = NULL;
    priv->icon = NULL;
    priv->category = KIRAN_NOTIFY_ICON_CATEGORY_APPLICATION_STATUS;
    priv->app_category = NULL;
    priv->has_alpha = TRUE;
    priv->has_desktop = TRUE;
}

static char *
latin1_to_utf8(const char *latin1)
{
    GString *str;
    const char *p;

    str = g_string_new(NULL);

    p = latin1;
    while (*p)
    {
        g_string_append_unichar(str, (gunichar)*p);
        ++p;
    }

    return g_string_free(str, FALSE);
}

static void
_get_wmclass(Display *xdisplay,
             Window xwindow,
             char **res_class,
             char **res_name)
{
    GdkDisplay *display;
    XClassHint ch;

    ch.res_name = NULL;
    ch.res_class = NULL;

    display = gdk_display_get_default();
    gdk_x11_display_error_trap_push(display);
    XGetClassHint(xdisplay, xwindow, &ch);
    gdk_x11_display_error_trap_pop_ignored(display);

    if (res_class)
        *res_class = NULL;

    if (res_name)
        *res_name = NULL;

    if (ch.res_name)
    {
        if (res_name)
            *res_name = latin1_to_utf8(ch.res_name);

        XFree(ch.res_name);
    }

    if (ch.res_class)
    {
        if (res_class)
            *res_class = latin1_to_utf8(ch.res_class);

        XFree(ch.res_class);
    }
}

static const char *
kiran_x11_tray_icon_get_id(KiranNotifyIcon *icon)
{
    KiranX11TrayIconPrivate *priv = KIRAN_X11_TRAY_ICON(icon)->priv;

    return priv->id;
}

static const char *
kiran_x11_tray_icon_get_name(KiranNotifyIcon *icon)
{
    KiranX11TrayIconPrivate *priv = KIRAN_X11_TRAY_ICON(icon)->priv;
    GdkDisplay *display;
    Atom utf8_string, atom, type;
    int result;
    int format;
    gulong nitems;
    gulong bytes_after;
    gchar *val;

    if (priv->name)
        return priv->name;

    display = gtk_widget_get_display(GTK_WIDGET(icon));

    utf8_string = gdk_x11_get_xatom_by_name_for_display(display, "UTF8_STRING");
    atom = gdk_x11_get_xatom_by_name_for_display(display, "_NET_WM_NAME");

    gdk_x11_display_error_trap_push(display);

    result = XGetWindowProperty(GDK_DISPLAY_XDISPLAY(display),
                                priv->icon_window,
                                atom,
                                0, G_MAXLONG,
                                False, utf8_string,
                                &type, &format, &nitems,
                                &bytes_after, (guchar **)&val);

    if (gdk_x11_display_error_trap_pop(display) || result != Success)
        return priv->name;

    if (type != utf8_string ||
        format != 8 ||
        nitems == 0)
    {
        if (val)
            XFree(val);
        return priv->name;
    }

    if (!g_utf8_validate(val, nitems, NULL))
    {
        XFree(val);
        return priv->name;
    }

    priv->name = g_strndup(val, nitems);

    XFree(val);

    return priv->name;
}

static KiranNotifyIconCategory
kiran_x11_tray_icon_get_category(KiranNotifyIcon *icon)
{
    KiranX11TrayIconPrivate *priv = KIRAN_X11_TRAY_ICON(icon)->priv;

    return priv->category;
}

static const char *
kiran_x11_tray_icon_get_app_category(KiranNotifyIcon *icon)
{
    KiranX11TrayIconPrivate *priv = KIRAN_X11_TRAY_ICON(icon)->priv;

    return priv->app_category;
}

static KiranNotifyIconWay
kiran_x11_tray_icon_get_way(KiranNotifyIcon *icon)
{
    return KIRAN_NOTIFY_ICON_WAY_X11;
}

static const char *
kiran_x11_tray_icon_get_icon(KiranNotifyIcon *icon)
{
    KiranX11TrayIconPrivate *priv = KIRAN_X11_TRAY_ICON(icon)->priv;

    return priv->icon;
}

GtkWidget *
kiran_x11_tray_icon_get_socket(KiranX11TrayIcon *icon)
{
    KiranX11TrayIconPrivate *priv = KIRAN_X11_TRAY_ICON(icon)->priv;

    return priv->socket;
}

Window
kiran_x11_tray_icon_get_icon_window(KiranX11TrayIcon *icon)
{
    KiranX11TrayIconPrivate *priv = KIRAN_X11_TRAY_ICON(icon)->priv;

    return priv->icon_window;
}

GtkWidget *
kiran_x11_tray_icon_new(Window icon_window)
{
    KiranX11TrayIcon *icon;
    XWindowAttributes window_attributes;
    Display *xdisplay;
    GdkDisplay *display;
    GdkScreen *screen;
    GdkVisual *visual;
    char *res_name = NULL;
    char *res_class = NULL;
    guint i;
    gint result;

    display = gdk_display_get_default();
    screen = gdk_display_get_default_screen(display);
    xdisplay = GDK_SCREEN_XDISPLAY(screen);

    if (!GDK_IS_X11_DISPLAY(display))
    {
        g_warning("kiran tray only works on X11");
        return NULL;
    }

    gdk_x11_display_error_trap_push(display);
    result = XGetWindowAttributes(xdisplay, icon_window,
                                  &window_attributes);
    gdk_x11_display_error_trap_pop_ignored(display);

    if (!result) /* 窗口已经销毁 */
        return NULL;

    visual = gdk_x11_screen_lookup_visual(screen,
                                          window_attributes.visual->visualid);

    if (!visual)
        return NULL;

    icon = reinterpret_cast<KiranX11TrayIcon *>(g_object_new(KIRAN_TYPE_X11_TRAY_ICON, NULL));
    icon->priv->icon_window = icon_window;

    _get_wmclass(GDK_DISPLAY_XDISPLAY(display),
                 icon_window,
                 &res_class,
                 &res_name);

    for (i = 0; i < G_N_ELEMENTS(wmclass_categories); i++)
    {
        if (g_strcmp0(res_class, wmclass_categories[i].wm_class) == 0 ||
            g_strcmp0(res_name, wmclass_categories[i].wm_class) == 0)
        {
            icon->priv->id = g_strdup(wmclass_categories[i].id);
            icon->priv->category = wmclass_categories[i].category;
            break;
        }
    }

    if (!icon->priv->id)
    {
        auto app = Kiran::AppManager::get_instance()->lookup_app_with_xid(icon_window);

        if (!app)
        {
            auto window = std::make_shared<Kiran::Window>(icon_window);
            app = Kiran::AppManager::get_instance()->lookup_app_with_window(window);
        }

        if (app)
        {
            const char *id = app->get_desktop_id().c_str();
            const char *name = app->get_name().c_str();
            const char *category = app->get_categories().c_str();
            const char *icon_name = NULL;

            if (app->get_icon())
                icon_name = app->get_icon()->to_string().c_str();

            icon->priv->id = g_strdup(id);
            icon->priv->icon = g_strdup(icon_name);
            icon->priv->name = g_strdup(name);
            icon->priv->app_category = g_strdup(category);
        }
        else
        {
            //未找到对应的desktop文件
            icon->priv->id = g_strdup(res_name);
            icon->priv->name = g_strdup(res_name);
            icon->priv->app_category = g_strdup("ApplicationStatus");
            icon->priv->icon = g_strdup_printf("%s/.config/kiran-tray/icon/%s.png",
                                               g_get_home_dir(),
                                               icon->priv->id);

            icon->priv->has_desktop = FALSE;
        }
    }
    else
    {
        g_free(res_name);
    }

    g_free(res_class);

    return GTK_WIDGET(icon);
}
