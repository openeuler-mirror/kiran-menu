#include "kiran-notify-icon.h"
#include "kiran-x11-tray-icon.h"
#include "kiran-x11-tray-socket.h"

struct _KiranX11TrayIconPrivate
{
    Window icon_window;
    gchar *id;
    gchar *name;
    KiranNotifyIconCategory category;
    GdkPixbuf *icon;
    GtkWidget *socket;
    gboolean has_alpha;
};

enum
{
    PROP_0,
    PROP_ORIENTATION
};

static void                    kiran_notify_icon_init (KiranNotifyIconInterface *iface);
static const char              *kiran_x11_tray_icon_get_id (KiranNotifyIcon *icon);
static const char              *kiran_x11_tray_icon_get_name (KiranNotifyIcon *icon);
static KiranNotifyIconCategory kiran_x11_tray_icon_get_category (KiranNotifyIcon *icon);
static KiranNotifyIconWay kiran_x11_tray_icon_get_way (KiranNotifyIcon *icon);
static GdkPixbuf               *kiran_x11_tray_icon_get_icon (KiranNotifyIcon *icon);

#define KIRAN_X11_TRAY_ICON_GET_PRIVATE(o) 	(G_TYPE_INSTANCE_GET_PRIVATE ((o), KIRAN_TYPE_X11_TRAY_ICON, KiranX11TrayIconPrivate))

G_DEFINE_TYPE_WITH_CODE (KiranX11TrayIcon, kiran_x11_tray_icon, GTK_TYPE_EVENT_BOX,
			 G_IMPLEMENT_INTERFACE (GTK_TYPE_ORIENTABLE, NULL)
			 G_IMPLEMENT_INTERFACE (KIRAN_TYPE_NOTIFY_ICON, kiran_notify_icon_init))

static void
kiran_notify_icon_init (KiranNotifyIconInterface *iface)
{
   iface->get_id = kiran_x11_tray_icon_get_id; 
   iface->get_name = kiran_x11_tray_icon_get_name; 
   iface->get_icon = kiran_x11_tray_icon_get_icon; 
   iface->get_category = kiran_x11_tray_icon_get_category; 
   iface->get_way = kiran_x11_tray_icon_get_way; 
}

static GObject *
kiran_x11_tray_icon_constructor (GType                  type,
                                 guint                  n_construct_properties,
                                 GObjectConstructParam *construct_properties)
{
    GObject               *obj;
    KiranX11TrayIcon      *icon;

    obj = G_OBJECT_CLASS (kiran_x11_tray_icon_parent_class)->constructor (type,
                                                                               n_construct_properties,
                                                                               construct_properties);
    icon = KIRAN_X11_TRAY_ICON (obj);
   
    gtk_widget_set_can_focus (GTK_WIDGET (icon),  TRUE);

    icon->priv->socket = kiran_x11_tray_socket_new ();
    gtk_container_add (GTK_CONTAINER (icon), icon->priv->socket);
    gtk_widget_show (icon->priv->socket);

    return obj;
}

static void
kiran_x11_tray_icon_finalize (GObject *object)
{
    KiranX11TrayIconPrivate *priv = KIRAN_X11_TRAY_ICON (object)->priv;

    if (priv->id)
	g_free (priv->id);
    if (priv->name)
	g_free (priv->name);
    if (priv->icon)
	g_object_unref (priv->icon);
}

static void
kiran_x11_tray_icon_get_property (GObject *object,
				  guint    property_id,
				  GValue   *value,
				  GParamSpec *pspec)
{
    switch (property_id)
    {
	case PROP_ORIENTATION:
	    g_value_set_enum (value, GTK_ORIENTATION_HORIZONTAL);
	    break;

	default:
	    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	    break;
    }
}

static void
kiran_x11_tray_icon_set_property (GObject *object,
		                  guint property_id,
				  const GValue *value,
				  GParamSpec *pspec)
{
    switch (property_id)
    {
	case PROP_ORIENTATION:
	    break;

	default:
	    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	    break;
    }
   
}

static gboolean
kiran_x11_tray_icon_draw (GtkWidget *widget,
                          cairo_t   *cr)
{
    KiranX11TrayIcon *icon = KIRAN_X11_TRAY_ICON (widget);
    KiranX11TrayIconPrivate *priv = icon->priv;
   
    GTK_WIDGET_CLASS (kiran_x11_tray_icon_parent_class)->draw (widget, cr);

    kiran_x11_tray_socket_draw_on_parent (KIRAN_X11_TRAY_SOCKET (priv->socket),
		   		          widget,
					  cr); 
    return TRUE;
}

static gboolean
kiran_x11_tray_icon_leave_notify_event (GtkWidget *widget,
					GdkEventCrossing *event)
{
    GtkStateFlags flags;

    flags = gtk_widget_get_state_flags (widget);
    flags = flags & ~GTK_STATE_FLAG_PRELIGHT;

    gtk_widget_set_state_flags (widget, flags, TRUE); 

    return FALSE;
}

static gboolean
kiran_x11_tray_icon_enter_notify_event (GtkWidget *widget,
					GdkEventCrossing *event)
{
    gtk_widget_grab_focus (widget);

    gtk_widget_set_state_flags (widget, GTK_STATE_FLAG_PRELIGHT, FALSE); 

    return FALSE;
}

static gboolean
kiran_x11_tray_icon_focus_out_event (GtkWidget *widget,
				    GdkEventFocus *event)
{
    GtkStateFlags flags;

    flags = gtk_widget_get_state_flags (widget);
    flags = flags & ~GTK_STATE_FLAG_PRELIGHT;
    gtk_widget_set_state_flags (widget, flags, TRUE); 

    return FALSE;
}

static gboolean
kiran_x11_tray_icon_focus_in_event (GtkWidget *widget,
                                   GdkEventFocus *event)
{
    gtk_widget_set_state_flags (widget, GTK_STATE_FLAG_PRELIGHT, FALSE);

    return FALSE;
}

static void
kiran_x11_tray_icon_class_init (KiranX11TrayIconClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

    gobject_class->constructor = kiran_x11_tray_icon_constructor;
    gobject_class->finalize = kiran_x11_tray_icon_finalize;
    gobject_class->get_property = kiran_x11_tray_icon_get_property;
    gobject_class->set_property = kiran_x11_tray_icon_set_property;
  
    widget_class->leave_notify_event = kiran_x11_tray_icon_leave_notify_event;
    widget_class->enter_notify_event = kiran_x11_tray_icon_enter_notify_event;
    widget_class->focus_out_event = kiran_x11_tray_icon_focus_out_event;
    widget_class->focus_in_event = kiran_x11_tray_icon_focus_in_event;
    widget_class->draw = kiran_x11_tray_icon_draw;

    g_type_class_add_private (gobject_class, sizeof (KiranX11TrayIconPrivate));
    g_object_class_override_property (gobject_class, PROP_ORIENTATION, "orientation");
}

static void
kiran_x11_tray_icon_init (KiranX11TrayIcon *self)
{
    KiranX11TrayIconPrivate *priv;

    priv = self->priv = KIRAN_X11_TRAY_ICON_GET_PRIVATE (self);
    priv->id = NULL;
    priv->name = NULL;
    priv->icon = NULL;
    priv->category = KIRAN_NOTIFY_ICON_CATEGORY_APPLICATION_STATUS;
    priv->has_alpha = TRUE;
    
}

static char *
latin1_to_utf8 (const char *latin1)
{
    GString *str;
    const char *p;
  
    str = g_string_new (NULL);
  
    p = latin1;
    while (*p)
    {
        g_string_append_unichar (str, (gunichar) *p);
        ++p;
    }
  
    return g_string_free (str, FALSE);
}

static void
_get_wmclass (Display *xdisplay,
              Window   xwindow,
              char   **res_class,
              char   **res_name)
{
    GdkDisplay *display;
    XClassHint ch;
  
    ch.res_name = NULL;
    ch.res_class = NULL;
  
    display = gdk_display_get_default ();
    gdk_x11_display_error_trap_push (display);
    XGetClassHint (xdisplay, xwindow, &ch);
    gdk_x11_display_error_trap_pop_ignored (display);
  
    if (res_class)
        *res_class = NULL;
  
    if (res_name)
        *res_name = NULL;
  
    if (ch.res_name)
    {
        if (res_name)
            *res_name = latin1_to_utf8 (ch.res_name);
  
        XFree (ch.res_name);
    }
  
    if (ch.res_class)
    {
        if (res_class)
          *res_class = latin1_to_utf8 (ch.res_class);
  
        XFree (ch.res_class);
    }
}

static const char *
kiran_x11_tray_icon_get_id (KiranNotifyIcon *icon)
{
    KiranX11TrayIconPrivate *priv = KIRAN_X11_TRAY_ICON (icon)->priv;

    return priv->id;
}

static const char *
kiran_x11_tray_icon_get_name (KiranNotifyIcon *icon)
{
    KiranX11TrayIconPrivate *priv = KIRAN_X11_TRAY_ICON (icon)->priv;
    GdkDisplay *display;
    Atom utf8_string, atom, type;
    int result;
    int format;
    gulong nitems;
    gulong bytes_after;
    gchar *val;

    if (priv->name)
	return priv->name;

    display = gtk_widget_get_display (GTK_WIDGET (icon));
  
    utf8_string = gdk_x11_get_xatom_by_name_for_display (display, "UTF8_STRING");
    atom = gdk_x11_get_xatom_by_name_for_display (display, "_NET_WM_NAME");
  
    gdk_x11_display_error_trap_push (display);
  
    result = XGetWindowProperty (GDK_DISPLAY_XDISPLAY (display),
                                 priv->icon_window,
                                 atom,
                                 0, G_MAXLONG,
                                 False, utf8_string,
                                 &type, &format, &nitems,
                                 &bytes_after, (guchar **)&val);
  
    if (gdk_x11_display_error_trap_pop (display) || result != Success)
        return priv->name;
  
    if (type != utf8_string ||
        format != 8 ||
        nitems == 0)
    {
        if (val)
            XFree (val);
        return priv->name;
    }
  
    if (!g_utf8_validate (val, nitems, NULL))
    {
        XFree (val);
        return priv->name;
    }
  
    priv->name = g_strndup (val, nitems);
  
    XFree (val);
  
    return priv->name;
}

static KiranNotifyIconCategory 
kiran_x11_tray_icon_get_category (KiranNotifyIcon *icon)
{
    KiranX11TrayIconPrivate *priv = KIRAN_X11_TRAY_ICON (icon)->priv;

    return priv->category;
}

static KiranNotifyIconWay
kiran_x11_tray_icon_get_way (KiranNotifyIcon *icon)
{
    return KIRAN_NOTIFY_ICON_WAY_X11;
}

static GdkPixbuf *
kiran_x11_tray_icon_get_icon (KiranNotifyIcon *icon)
{
    KiranX11TrayIconPrivate *priv = KIRAN_X11_TRAY_ICON (icon)->priv;

    return priv->icon;
}

GtkWidget *
kiran_x11_tray_icon_get_socket (KiranX11TrayIcon *icon)
{
    KiranX11TrayIconPrivate *priv = KIRAN_X11_TRAY_ICON (icon)->priv;

    return priv->socket;
}

Window 	
kiran_x11_tray_icon_get_icon_window (KiranX11TrayIcon *icon)
{
    KiranX11TrayIconPrivate *priv = KIRAN_X11_TRAY_ICON (icon)->priv;

    return priv->icon_window;
}

GtkWidget *
kiran_x11_tray_icon_new (Window icon_window)
{
    KiranX11TrayIcon *icon;
    GdkDisplay *display; 
    char *res_name = NULL;
    char *res_class = NULL;
    guint i;

    icon = g_object_new (KIRAN_TYPE_X11_TRAY_ICON, NULL);
    icon->priv->icon_window = icon_window;


    display = gtk_widget_get_display (GTK_WIDGET (icon));
    _get_wmclass (GDK_DISPLAY_XDISPLAY (display),
            icon_window,
            &res_class,
            &res_name);
    
    for (i = 0; i < G_N_ELEMENTS (wmclass_categories); i++)
    {
        if (g_strcmp0 (res_class, wmclass_categories[i].wm_class) == 0 ||
	    g_strcmp0 (res_name, wmclass_categories[i].wm_class) == 0)
        {
            icon->priv->id = g_strdup (wmclass_categories[i].id);
    	    icon->priv->category = wmclass_categories[i].category;
	    break;
        }
    }
    
    if (!icon->priv->id)
        icon->priv->id = res_name;
    else
        g_free (res_name);
    
    g_free (res_class);

    return GTK_WIDGET (icon);
}
