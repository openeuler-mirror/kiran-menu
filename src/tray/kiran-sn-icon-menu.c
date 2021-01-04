#include <libdbusmenu-glib/dbusmenu-glib.h>
#include "kiran-sn-icon-menu.h"

struct _KiranSnIconMenuPrivate
{
    gchar *bus_name;
    gchar *object_path;

    DbusmenuClient *client;
};

enum
{
    PROP_0,
    PROP_BUS_NAME,
    PROP_OBJECT_PATH,
    LAST_PROP
};

static GParamSpec *properties[LAST_PROP] = { NULL };

G_DEFINE_TYPE_WITH_PRIVATE (KiranSnIconMenu, kiran_sn_icon_menu, GTK_TYPE_MENU)

static void
activate_cb (GtkWidget  *widget,
             DbusmenuMenuitem *item)
{
  if (gtk_menu_item_get_submenu (GTK_MENU_ITEM (widget)) != NULL)
      return;

  dbusmenu_menuitem_handle_event (item, 
		  		  DBUSMENU_MENUITEM_EVENT_ACTIVATED, 
				  g_variant_new ("v", g_variant_new_int32 (0)), 
				  gtk_get_current_event_time ());
}


static GdkPixbuf *
creat_pxibuf_from_variant (GVariant *variant)
{
    gsize length;
    const guchar *data;
    GInputStream *stream;
    GdkPixbuf *pixbuf;
    GError *error;
  
    data = g_variant_get_fixed_array (variant, &length, sizeof (guchar));
  
    if (length == 0)
      return NULL;
  
    stream = g_memory_input_stream_new_from_data (data, length, NULL);
  
    if (stream == NULL)
      return NULL;
  
    error = NULL;
    pixbuf = gdk_pixbuf_new_from_stream (stream, NULL, &error);
    g_object_unref (stream);
  
    if (error != NULL)
    {
        g_warning ("Unable to build GdkPixbuf from icon data: %s", error->message);
        g_error_free (error);
    }
  
    return pixbuf;
}

static GtkWidget *
create_widget_from_menuitem (DbusmenuMenuitem *item)
{
    GtkWidget *gmi = NULL;
    GdkPixbuf   *icon_data = NULL;
    const gchar *label = dbusmenu_menuitem_property_get (item, DBUSMENU_MENUITEM_PROP_LABEL);
    const gchar *type = dbusmenu_menuitem_property_get (item, DBUSMENU_MENUITEM_PROP_TYPE);
    const gchar *toggle_type = dbusmenu_menuitem_property_get (item, DBUSMENU_MENUITEM_PROP_TOGGLE_TYPE);
    const gchar *icon_name = dbusmenu_menuitem_property_get (item, DBUSMENU_MENUITEM_PROP_ICON_NAME);
    const gchar *children_display = dbusmenu_menuitem_property_get (item, DBUSMENU_MENUITEM_PROP_CHILD_DISPLAY);
    gint toggle_state = dbusmenu_menuitem_property_get_int (item, DBUSMENU_MENUITEM_PROP_TOGGLE_STATE);
    gboolean enabled = dbusmenu_menuitem_property_get_bool (item, DBUSMENU_MENUITEM_PROP_ENABLED);
    gboolean visible = dbusmenu_menuitem_property_get_bool (item, DBUSMENU_MENUITEM_PROP_VISIBLE);

    GVariant *icon_data_variant = dbusmenu_menuitem_property_get_variant (item, DBUSMENU_MENUITEM_PROP_ICON_DATA);


    if (icon_data_variant)
        icon_data = creat_pxibuf_from_variant (icon_data_variant);

    if (g_strcmp0 (type, "separator") == 0)
    {
	gmi = gtk_separator_menu_item_new ();	
    }
    else
    {
	if (g_strcmp0 (toggle_type, "checkmark") == 0)
	{
	    gmi = gtk_check_menu_item_new ();
	}	
	else if (g_strcmp0 (toggle_type, "radio") == 0)
	{
	    AtkObject *atk_obj;

	    gmi = gtk_check_menu_item_new ();
	    gtk_check_menu_item_set_draw_as_radio (GTK_CHECK_MENU_ITEM(gmi),TRUE);
	    atk_obj = gtk_widget_get_accessible (gmi);
	    atk_object_set_role (atk_obj,ATK_ROLE_RADIO_MENU_ITEM);
	}
	else
	{
	    gmi = gtk_menu_item_new ();
	}

	if (g_strcmp0 (children_display, "submenu") == 0)
        {
             GtkWidget *submenu;
	     GList *child;
             GList *children = dbusmenu_menuitem_get_children (item);

             submenu = gtk_menu_new ();
             gtk_menu_item_set_submenu (GTK_MENU_ITEM (gmi), submenu);

	     for (child = children; child; child = child->next)
             {
                 GtkWidget *gmi = create_widget_from_menuitem (child->data);
         
                 gtk_menu_shell_append (GTK_MENU_SHELL (submenu), gmi);
                 gtk_widget_show (gmi);
         
                 g_signal_connect (gmi,
                                   "activate",
                                   G_CALLBACK (activate_cb),
                                   child->data);
             }
         
	     gtk_widget_show (submenu);
        }

	if (toggle_state != DBUSMENU_MENUITEM_TOGGLE_STATE_UNKNOWN && 
			GTK_IS_CHECK_MENU_ITEM (gmi))
        {
            GtkCheckMenuItem *check;

            check = GTK_CHECK_MENU_ITEM (gmi);

            if (toggle_state == DBUSMENU_MENUITEM_TOGGLE_STATE_CHECKED)
                gtk_check_menu_item_set_active (check, TRUE);
            else if (toggle_state == DBUSMENU_MENUITEM_TOGGLE_STATE_UNCHECKED)
                gtk_check_menu_item_set_active (check, FALSE);
        }

	gtk_menu_item_set_use_underline (GTK_MENU_ITEM (gmi), TRUE);
        gtk_menu_item_set_label (GTK_MENU_ITEM (gmi), label);
    }

    gtk_widget_set_sensitive (gmi, enabled);
    gtk_widget_set_visible (gmi, visible);

    if (icon_data)
	g_object_unref (icon_data);

    g_object_ref_sink (gmi);

    return gmi;
}

static void
layout_updated_cb (DbusmenuClient *client,
		   gpointer user_data)
{
    KiranSnIconMenu *menu;
    DbusmenuMenuitem  *root = dbusmenu_client_get_root (client);
    GList *child;
    GList *children = dbusmenu_menuitem_get_children (root);

    menu = KIRAN_SN_ICON_MENU (user_data);

    for (child = children; child; child = child->next)
    {
	GtkWidget *gmi = create_widget_from_menuitem (child->data);

	gtk_menu_shell_append (GTK_MENU_SHELL (menu), gmi);
	gtk_widget_show (gmi);

	g_signal_connect (gmi, 
	    	          "activate",
                          G_CALLBACK (activate_cb), 
	    	          child->data);
    }
}

static void
kiran_sn_icon_menu_constructed (GObject *object)
{
    KiranSnIconMenu *menu;
    KiranSnIconMenuPrivate *priv;
    GtkWidget *toplevel;
    GdkScreen *screen;
    GdkVisual *visual;
    GtkStyleContext *context;


    G_OBJECT_CLASS (kiran_sn_icon_menu_parent_class)->constructed (object);

    menu = KIRAN_SN_ICON_MENU (object);
    priv = kiran_sn_icon_menu_get_instance_private (menu);

    /*Set up theme and transparency support*/
    toplevel = gtk_widget_get_toplevel(GTK_WIDGET(menu));
    /* Fix any failures of compiz/other wm's to communicate with gtk for transparency */
    screen = gtk_widget_get_screen(GTK_WIDGET(toplevel));
    visual = gdk_screen_get_rgba_visual(screen);
    gtk_widget_set_visual(GTK_WIDGET(toplevel), visual);
    /* Set menu and it's toplevel window to follow panel theme */
    context = gtk_widget_get_style_context (GTK_WIDGET(toplevel));
    gtk_style_context_add_class(context,"mate-panel-menu-bar");

    priv->client = dbusmenu_client_new (priv->bus_name,
		                           priv->object_path);

    g_signal_connect (priv->client, DBUSMENU_CLIENT_SIGNAL_LAYOUT_UPDATED, G_CALLBACK (layout_updated_cb), menu);
}

static void
kiran_sn_icon_menu_set_property (GObject      *object,
                                 guint         property_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
    KiranSnIconMenu *menu;
    KiranSnIconMenuPrivate *priv;
  
    menu = KIRAN_SN_ICON_MENU (object);
    priv = kiran_sn_icon_menu_get_instance_private (menu);
  
    switch (property_id)
      {
        case PROP_BUS_NAME:
          priv->bus_name = g_value_dup_string (value);
          break;
  
        case PROP_OBJECT_PATH:
          priv->object_path = g_value_dup_string (value);
          break;
  
        default:
          G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
          break;
      }
}

static void
kiran_sn_icon_menu_finalize (GObject *object)
{
    KiranSnIconMenu *menu;
    KiranSnIconMenuPrivate *priv;
  
    menu = KIRAN_SN_ICON_MENU (object);
    priv = kiran_sn_icon_menu_get_instance_private (menu);


    g_free (priv->bus_name);
    g_free (priv->object_path);
    g_object_unref (priv->client);

    G_OBJECT_CLASS (kiran_sn_icon_menu_parent_class)->finalize (object);
}

static void
kiran_sn_icon_menu_class_init (KiranSnIconMenuClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->constructed = kiran_sn_icon_menu_constructed;
    gobject_class->set_property = kiran_sn_icon_menu_set_property;
    gobject_class->finalize = kiran_sn_icon_menu_finalize;

    properties[PROP_BUS_NAME] = 
	g_param_spec_string ("bus-name",
		       	     "bus-name",	
		       	     "bus-name",	
			     NULL,
			     G_PARAM_CONSTRUCT_ONLY |
			     G_PARAM_WRITABLE |
			     G_PARAM_STATIC_STRINGS);

    properties[PROP_OBJECT_PATH] = 
	g_param_spec_string ("object-path",
		       	     "object-path",	
		       	     "object-path",	
			     NULL,
			     G_PARAM_CONSTRUCT_ONLY |
			     G_PARAM_WRITABLE |
			     G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (gobject_class, LAST_PROP, properties);
}

static void
kiran_sn_icon_menu_init (KiranSnIconMenu *self)
{
    KiranSnIconMenuPrivate *priv;
  
    priv = self->priv = kiran_sn_icon_menu_get_instance_private (self);
}

GtkMenu *
kiran_sn_icon_menu_new (gchar *bus_name,
		        gchar *object_path)
{
    return g_object_new (KIRAN_TYPE_SN_ICON_MENU, 
		         "bus-name", bus_name,
			 "object-path", object_path,
			 NULL);
}
