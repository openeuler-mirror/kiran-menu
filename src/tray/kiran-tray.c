#include <gdk/gdkx.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include "kiran-tray.h"
#include "kiran-notify-icon.h"
#include "kiran-x11-tray-manager.h"
#include "kiran-notify-icon-window.h"
#include "kiran-sn-tray-manager.h"
#include "kiran-x11-tray-socket.h"

#define ROOT_NODE_NAME "apps"
#define APP_NODE_NAME "app"
#define APP_ID_NODE_NAME "id" 
#define APP_ICON_NODE_NAME "icon" /*应用图标 */
#define APP_TITLE_NODE_NAME "title" /*应用名称*/
#define APP_CATEGORY_NODE_NAME "category" /*应用分类 */


struct _KiranTrayPrivate
{
    GtkWidget *icons_win_button;
    GtkWidget *icons_win;
    GSList *icons;
    GSList *managers;
    GSettings *settings;
    KiranTrayLoaction location;
};

static GObject *kiran_tray_constructor (GType		     type,
					guint		     n_construct_properties,
					GObjectConstructParam *construct_properties);
static void kiran_tray_realize (GtkWidget *widget);
static void kiran_tray_unrealize (GtkWidget *widget);

static void kiran_tray_add_manager (KiranTray *tray, KiranTrayManager *manager);
static void kiran_tray_icons_refresh (KiranTray *tray);
static void position_notify_icon_window (KiranTray *tray, gboolean change_y);

G_DEFINE_TYPE_WITH_PRIVATE (KiranTray, kiran_tray, GTK_TYPE_BOX)

static void
kiran_tray_set_icon_size (KiranTray *tray)
{
    KiranTrayPrivate *priv;
    GSList *iterator = NULL;
    gint icon_size = 0;

    priv = tray->priv;
    icon_size = g_settings_get_int (priv->settings, KEY_TRAY_ICON_SIZE);

    for (iterator = priv->managers; iterator; iterator = iterator->next)
    {
	kiran_tray_manager_set_icon_size (iterator->data, icon_size);
    }
}

static void
kiran_tray_set_icon_padding (KiranTray *tray)
{
    KiranTrayPrivate *priv;
    gint icon_padding = 2;

    priv = tray->priv;
    icon_padding = g_settings_get_int (priv->settings, KEY_TRAY_ICON_PADDING);

    gtk_box_set_spacing (GTK_BOX (tray), icon_padding);
}

static KiranNotifyIcon *
kiran_tray_find_icon_by_id (KiranTray *tray,
			    const gchar *id)
{
    KiranTrayPrivate *priv;
    GSList *iterator = NULL;

    priv = tray->priv;

    for (iterator = priv->icons; iterator; iterator = iterator->next)
    {
 	const gchar *icon_id;

	if (!KIRAN_IS_NOTIFY_ICON (iterator->data))
            continue;
	
	icon_id = kiran_notify_icon_get_id (KIRAN_NOTIFY_ICON (iterator->data));
	if (g_strcmp0 (icon_id, id) == 0)
        {
            return iterator->data;
        }
    }

    return NULL;
}

static void
gsettings_changed_panel_icon_ids (GSettings    *settings,
                                  gchar        *key,
                                  KiranTray    *tray)
{
    KiranTrayPrivate *priv = tray->priv;
    KiranNotifyIcon *icon;
    gchar **panel_icon_ids;
    gboolean find = TRUE;
    gint i;
    GList *children;
    GList *child;

    panel_icon_ids = g_settings_get_strv (settings, KEY_PANEL_ICON_IDS);
    children = gtk_container_get_children (GTK_CONTAINER (tray));

    if (panel_icon_ids)
    {
        for (i = 0; panel_icon_ids && panel_icon_ids[i]; i++)
	{
	    icon = kiran_tray_find_icon_by_id (tray, panel_icon_ids[i]);	

	    if (icon)
	    {
	        find = FALSE; 

	        for (child = children; child; child = child->next)
	        {
	            const gchar *id;
	            	
	            if (!KIRAN_IS_NOTIFY_ICON (child->data))
	                continue;

	            id = kiran_notify_icon_get_id (KIRAN_NOTIFY_ICON (child->data));

	            if (g_strcmp0 (id, panel_icon_ids[i]) == 0)
	            {
	                find = TRUE;
	                break;
	            }
	        }

	        //remove from tray to panel
	        if (!find)
	        {
		    KiranNotifyIconWay way = kiran_notify_icon_get_way (icon);

		    if (way == KIRAN_NOTIFY_ICON_WAY_X11)
	                kiran_notify_icon_window_remove_icon (KIRAN_NOTIFY_ICON_WINDOW (priv->icons_win), icon);
		    else if (way == KIRAN_NOTIFY_ICON_WAY_DBUS)
		    {
			g_object_ref (icon);
	                kiran_notify_icon_window_remove_icon (KIRAN_NOTIFY_ICON_WINDOW (priv->icons_win), icon);
        		gtk_box_pack_start (GTK_BOX (tray), GTK_WIDGET (icon), FALSE, TRUE, 0);
			kiran_tray_icons_refresh (tray);
		    }

                    if (kiran_notify_icon_window_get_icons_number (KIRAN_NOTIFY_ICON_WINDOW (priv->icons_win)) <= 0)
                    {
	    		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->icons_win_button), FALSE);
                	gtk_widget_hide (priv->icons_win);
            		gtk_widget_hide (priv->icons_win_button);
                    }
	        }
	    }
	}

	for (child = children; child; child = child->next)
	{
	    const gchar *id;
                        
            if (!KIRAN_IS_NOTIFY_ICON (child->data))
                continue;

            id = kiran_notify_icon_get_id (KIRAN_NOTIFY_ICON (child->data));
	    find = FALSE; 
            for (i = 0; panel_icon_ids && panel_icon_ids[i]; i++)
	    {
	        if (g_strcmp0 (id, panel_icon_ids[i]) == 0)
	        {
	            find = TRUE;
	            break;
	        }
	    }

	    //remove from panel to tray
	    if (!find)
	    {
		KiranNotifyIconWay way = kiran_notify_icon_get_way (child->data);

		if (way == KIRAN_NOTIFY_ICON_WAY_X11)
		    gtk_container_remove (GTK_CONTAINER (tray), GTK_WIDGET (child->data));
		else if (way == KIRAN_NOTIFY_ICON_WAY_DBUS)
		{
		    g_object_ref (child->data);
		    gtk_container_remove (GTK_CONTAINER (tray), GTK_WIDGET (child->data));
                    kiran_notify_icon_window_add_icon (KIRAN_NOTIFY_ICON_WINDOW (priv->icons_win), child->data); 
                    gtk_widget_show (priv->icons_win_button);
		}
	    }
	}
	
	position_notify_icon_window (tray, FALSE);

        g_strfreev (panel_icon_ids);
    }
}

static void
gsettings_changed_panel_icon_size (GSettings    *settings,
                                   gchar        *key,
                                   KiranTray    *tray)
{
    kiran_tray_set_icon_size (tray);
}

static void
gsettings_changed_panel_icon_padding (GSettings    *settings,
                                      gchar        *key,
                                      KiranTray    *tray)
{
   
    kiran_tray_set_icon_padding (tray);
}

static void
kiran_tray_class_init (KiranTrayClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

    gobject_class->constructor = kiran_tray_constructor;
    widget_class->realize = kiran_tray_realize;
    widget_class->unrealize = kiran_tray_unrealize;
}

static void
kiran_tray_init (KiranTray *self)
{
    GdkDisplay *display;
    GtkCssProvider *provider;
    GdkScreen *screen;
    KiranTrayPrivate *priv;

    priv = self->priv = kiran_tray_get_instance_private (self);
    priv->settings = g_settings_new (KIRAN_TRAY_SCHEMA);
    g_signal_connect (priv->settings, "changed::" KEY_PANEL_ICON_IDS, G_CALLBACK (gsettings_changed_panel_icon_ids), self);
    g_signal_connect (priv->settings, "changed::" KEY_TRAY_ICON_SIZE, G_CALLBACK (gsettings_changed_panel_icon_size), self);
    g_signal_connect (priv->settings, "changed::" KEY_TRAY_ICON_PADDING, G_CALLBACK (gsettings_changed_panel_icon_padding), self);
    priv->location = APPLET_ORIENT_UP;

    provider = gtk_css_provider_new ();
    display = gdk_display_get_default ();
    screen = gdk_display_get_default_screen (display);
    gtk_style_context_add_provider_for_screen (screen,
                                             GTK_STYLE_PROVIDER(provider),
                                             GTK_STYLE_PROVIDER_PRIORITY_USER);
    gtk_css_provider_load_from_resource (provider, "/kiran-applet/css/applet.css");


    gtk_widget_set_halign(GTK_WIDGET(self), GTK_ALIGN_START);
    gtk_widget_set_hexpand(GTK_WIDGET(self), FALSE);
    g_object_unref (provider);
}

static void
kiran_tray_icons_refresh (KiranTray *tray)
{
    GList *children;
    GList *child;
    gint col = 0;

    children = gtk_container_get_children (GTK_CONTAINER (tray));
    children = g_list_sort (children, (GCompareFunc)kiran_notify_icon_compare);

    for (child = children; child; child = child->next)
    {
        gtk_container_child_set (GTK_CONTAINER (tray),
                              child->data,
                              "position", col,
                              NULL);
	col++;
    }
}

static void
position_notify_icon_window (KiranTray *tray, gboolean change_y)
{
    GtkAllocation   allocation;
    GdkDisplay     *display;
    GdkScreen      *screen;
    GdkRectangle    monitor;
    GdkGravity      gravity = GDK_GRAVITY_NORTH_WEST;
    int             button_w, button_h;
    int             x, y;
    int             w, h;
    int             i, n;
    gboolean        found_monitor = FALSE;
    KiranTrayPrivate *priv = tray->priv;

    if (!GDK_IS_X11_DISPLAY (gdk_display_get_default ()))
        return;

    if (!gtk_widget_is_visible (priv->icons_win))
	return;

    /* Get root origin of the toggle button, and position above that. */
    gdk_window_get_origin (gtk_widget_get_window (priv->icons_win_button),
                           &x, &y);

    gtk_widget_get_size_request (GTK_WIDGET (priv->icons_win), &w, &h);

    gtk_widget_get_allocation (priv->icons_win_button, &allocation);
    button_w = allocation.width;
    button_h = allocation.height;

    screen = gtk_window_get_screen (GTK_WINDOW (priv->icons_win));
    display = gdk_screen_get_display (screen);

    n = gdk_display_get_n_monitors (display);
    for (i = 0; i < n; i++) {
            gdk_monitor_get_geometry (gdk_display_get_monitor (display, i), &monitor);
            if (x >= monitor.x && x <= monitor.x + monitor.width &&
                y >= monitor.y && y <= monitor.y + monitor.height) {
                    found_monitor = TRUE;
                    break;
            }
    }

    if (!found_monitor) {
            /* eek, we should be on one of those xinerama
               monitors */
            monitor.x = 0;
            monitor.y = 0;
            monitor.width = WidthOfScreen (gdk_x11_screen_get_xscreen (screen));
            monitor.height = HeightOfScreen (gdk_x11_screen_get_xscreen (screen));
    }

        /* Based on panel orientation, position the popup.
     * Ignore window gravity since the window is undecorated.
     * The orientations are all named backward from what
     * I expected.
     */
    switch (priv->location) {
    case APPLET_ORIENT_RIGHT:
            x += button_w;
            if ((y + h) > monitor.y + monitor.height)
                    y -= (y + h) - (monitor.y + monitor.height);

            if ((y + h) > (monitor.height / 2))
                    gravity = GDK_GRAVITY_SOUTH_WEST;
            else
                    gravity = GDK_GRAVITY_NORTH_WEST;

            break;
    case APPLET_ORIENT_LEFT:
            x -= w;
            if ((y + h) > monitor.y + monitor.height)
                    y -= (y + h) - (monitor.y + monitor.height);

            if ((y + h) > (monitor.height / 2))
                    gravity = GDK_GRAVITY_SOUTH_EAST;
            else
                    gravity = GDK_GRAVITY_NORTH_EAST;

            break;
    case APPLET_ORIENT_DOWN:
            y += button_h;
            if ((x + w) > monitor.x + monitor.width)
                    x -= (x + w) - (monitor.x + monitor.width);

	    x -= (w - button_w) / 2;
	    y += 5;
            gravity = GDK_GRAVITY_NORTH_WEST;

            break;
    case APPLET_ORIENT_UP:
            y -= h;
            if ((x + w) > monitor.x + monitor.width)
                    x -= (x + w) - (monitor.x + monitor.width);

	    x -= (w - button_w) / 2 ;
	    y -= 5;

            gravity = GDK_GRAVITY_SOUTH_WEST;

            break;
    }

    //避免移出屏幕外
    if (x < 0)
	x = 0;

    if (y < 0)
	y = 0;

    if (change_y)
        gtk_window_move (GTK_WINDOW (priv->icons_win), x, y);
    else
    {
	gint root_x;
	gint root_y;

	gtk_window_get_position (GTK_WINDOW (priv->icons_win), &root_x, &root_y);
	gtk_window_move (GTK_WINDOW (priv->icons_win), x, root_y);
    }

    gtk_window_set_gravity (GTK_WINDOW (priv->icons_win), gravity);
}

static void
icons_win_button_toggled_cb (GtkWidget *widget,
			     gpointer  user_data)
{
    KiranTray *tray = KIRAN_TRAY (user_data);
    KiranTrayPrivate *priv = tray->priv;

    if (!gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (tray->priv->icons_win_button)))
    {
	gtk_widget_hide (priv->icons_win);
    }
    else
    {
	gtk_widget_show (priv->icons_win);
	position_notify_icon_window (tray, TRUE);
    }
}

static void
icons_win_hide_cb (GtkWidget *widget,
                   gpointer  user_data)
{
    KiranTray *tray = KIRAN_TRAY (user_data);

    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (tray->priv->icons_win_button), FALSE);
}

static GObject *
kiran_tray_constructor (GType		     type,
			guint		     n_construct_properties,
			GObjectConstructParam *construct_properties)
{
    GObject *obj;
    KiranTray *tray;

    obj = G_OBJECT_CLASS (kiran_tray_parent_class)->constructor (type,
		    						 n_construct_properties,
								 construct_properties);
    tray = KIRAN_TRAY (obj);
    gtk_box_set_homogeneous (GTK_BOX (tray), FALSE);
    gtk_widget_set_name (GTK_WIDGET (tray), "trayBox");
    kiran_tray_set_icon_padding (tray);

    tray->priv->icons_win = kiran_notify_icon_window_new ();
    tray->priv->icons_win_button = gtk_toggle_button_new ();
    gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON (tray->priv->icons_win_button),TRUE);
    gtk_widget_set_name (tray->priv->icons_win_button, "iconWinButton");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (tray->priv->icons_win_button),  FALSE);

    g_signal_connect (tray->priv->icons_win_button, "toggled", G_CALLBACK (icons_win_button_toggled_cb), tray);
    g_signal_connect (tray->priv->icons_win, "hide", G_CALLBACK (icons_win_hide_cb), tray);

    gtk_box_pack_start (GTK_BOX (tray), GTK_WIDGET (tray->priv->icons_win_button), FALSE, TRUE, 0);

    return obj;
}

static void
kiran_tray_realize (GtkWidget *widget)
{
    KiranTray *tray =  KIRAN_TRAY (widget);
    KiranTrayManager *manager;

    GTK_WIDGET_CLASS (kiran_tray_parent_class)->realize (widget);

    manager = kiran_x11_tray_manager_new (gtk_widget_get_screen (widget));

    kiran_tray_add_manager (tray, manager);

    manager = kiran_sn_tray_manager_new ();
    kiran_tray_add_manager (tray, manager);

    kiran_tray_set_icon_size (tray);
}

static void
kiran_tray_unrealize (GtkWidget *widget)
{
    KiranTrayPrivate *priv = KIRAN_TRAY (widget)->priv;

    if (priv->managers)
    {
	g_slist_free_full (priv->managers, g_object_unref);
	priv->managers = NULL;
    }

    g_clear_pointer (&priv->icons, g_slist_free);

    GTK_WIDGET_CLASS (kiran_tray_parent_class)->unrealize (widget);
}

static gint
kiran_tray_get_icon_type (GSettings *settings, const char *id)
{
    gchar **panel_icon_ids;
    gchar **hide_icon_ids;
    gint ret = ICON_SHOW_IN_WINDOW;
    gint i;

    if (!id)
	return ret;

    hide_icon_ids = g_settings_get_strv (settings, KEY_HIDE_ICON_IDS);
    if (hide_icon_ids)
    {
        for (i = 0; hide_icon_ids && hide_icon_ids[i]; i++)
	{
	    if (g_strcmp0 (id, hide_icon_ids[i]) == 0)
	    {
        	g_strfreev (hide_icon_ids);
		return  ICON_NOT_SHOW;
	    }
	}
        g_strfreev (hide_icon_ids);
    }

    panel_icon_ids = g_settings_get_strv (settings, KEY_PANEL_ICON_IDS);
    if (panel_icon_ids)
    {
        for (i = 0; panel_icon_ids && panel_icon_ids[i]; i++)
	{
	    if (g_strcmp0 (id, panel_icon_ids[i]) == 0)
	    {
		ret = ICON_SHOW_IN_PANEL;
	    }
	}
        g_strfreev (panel_icon_ids);
    }


    return ret;
}

static xmlNodePtr
create_app_node (KiranNotifyIcon *icon)
{
    xmlNodePtr node;
    KiranNotifyIconCategory icon_cate;
    const char *id;
    const char *icon_name;
    const char *name;
    const char *app_category;

    id = kiran_notify_icon_get_id (icon);
    icon_name = kiran_notify_icon_get_icon (icon);
    name = kiran_notify_icon_get_name (icon);
    app_category = kiran_notify_icon_get_app_category (icon);

    node = xmlNewNode (NULL, (const xmlChar *)APP_NODE_NAME);
    xmlNewTextChild (node, NULL, (const xmlChar *)APP_ID_NODE_NAME, (const xmlChar *)id);
    xmlNewTextChild (node, NULL, (const xmlChar *)APP_ICON_NODE_NAME, (const xmlChar *)icon_name);
    xmlNewTextChild (node, NULL, (const xmlChar *)APP_TITLE_NODE_NAME, (const xmlChar *)name);
    xmlNewTextChild (node, NULL, (const xmlChar *)APP_CATEGORY_NODE_NAME, (const xmlChar *)app_category);

    return node;
} 

static void
record_tray_notify_icon (KiranNotifyIcon  *icon)
{
    gchar *docname = NULL;
    xmlDocPtr doc;
    xmlNodePtr root_node;
    xmlNodePtr node;
    const char *id;

    id = kiran_notify_icon_get_id (icon);

    docname = g_strdup_printf ("%s/.config/kiran-tray/apps.xml", g_get_home_dir ());
    if (docname == NULL)
	return;

    if (!g_file_test (docname, G_FILE_TEST_EXISTS))
    {
	//文件不存在
	gchar *dir = NULL;
	
	dir = g_strdup_printf ("%s/.config", g_get_home_dir ());
	if (dir == NULL)
	{
    	    g_free (docname);
	    return;
	}

        if (!g_file_test (dir, G_FILE_TEST_EXISTS))
	{
	    g_mkdir (dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	}
	free (dir);

	dir = g_strdup_printf ("%s/.config/kiran-tray", g_get_home_dir ());
	if (dir == NULL)
	{
    	    g_free (docname);
	    return;
	}

        if (!g_file_test (dir, G_FILE_TEST_EXISTS))
	{
	    g_mkdir (dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	}
	free (dir);
    }

    doc = NULL;
    if (g_file_test (docname, G_FILE_TEST_EXISTS))
        doc = xmlReadFile(docname, NULL, XML_PARSE_NOBLANKS);

    if (doc == NULL)
    {
	doc = xmlNewDoc ((const xmlChar *)"1.0");
	root_node = xmlNewNode (NULL, (const xmlChar *)ROOT_NODE_NAME);
 	node = create_app_node (icon);
	xmlAddChild (root_node, node);

	xmlDocSetRootElement (doc, root_node);
	xmlSaveFormatFileEnc (docname, doc, "UTF-8", 1);
    }
    else
    {
	root_node = xmlDocGetRootElement (doc);
	if (root_node == NULL)
	{
	    if (g_remove (docname) == 0)
	    {
	        record_tray_notify_icon (icon);
	    }
	}
	else
	{
	    if (xmlStrcmp(root_node->name, (const xmlChar *)ROOT_NODE_NAME))
            {
		record_tray_notify_icon (icon);
            }
	    else
	    {
		gboolean is_exist = FALSE;

                node = root_node->xmlChildrenNode;
                while (node != NULL)
                {
		    if (!xmlStrcmp(node->name, (const xmlChar *)APP_NODE_NAME))
		    {
                        xmlNodePtr id_node;

			id_node = node->xmlChildrenNode;
			while (id_node != NULL)
			{
		    	    if (!xmlStrcmp(id_node->name, (const xmlChar *)APP_ID_NODE_NAME))
		    	    {
                                if (!xmlStrcmp (xmlNodeGetContent (id_node), (const xmlChar *)id))
				{
				    is_exist = TRUE;
				    break;
				}
                            }
			    id_node = id_node->next;
			}
		    }
                
                    node = node->next;
                }

		if (!is_exist)
		{
		    node = create_app_node (icon);
                    xmlAddChild (root_node, node);
	            xmlSaveFormatFileEnc (docname, doc, "UTF-8", 1);
		}
	    }
	}
    }

    //关闭文档指针，并清除文档中所有节点动态申请的内存
    xmlFreeDoc (doc);
    g_free (docname);
}

static void
kiran_tray_notify_icon_map_callback (GtkWidget *widget,
			             gpointer  user_data)
{
    KiranNotifyIcon *icon = KIRAN_NOTIFY_ICON (widget);

    /* 当窗口保存图标时才进行记录 */
    if (kiran_notify_icon_get_icon (icon))
    {
        record_tray_notify_icon (icon);
    }
}

static void
kiran_tray_notify_icon_added (KiranTrayManager *manager,
			      KiranNotifyIcon  *icon,
			      KiranTray        *tray)
{
    KiranTrayPrivate *priv;
    const char *id;
    gint type;

    g_return_if_fail (KIRAN_IS_TRAY_MANAGER (manager));
    g_return_if_fail (KIRAN_IS_NOTIFY_ICON (icon));
    g_return_if_fail (KIRAN_IS_TRAY (tray));

    priv = tray->priv;

    id = kiran_notify_icon_get_id (icon);
    type = kiran_tray_get_icon_type (priv->settings, id);

    /* 系统已知的通知图标如声音，网络，电池等不需要记录 */
    if (kiran_notify_icon_get_category (icon) != KIRAN_NOTIFY_ICON_CATEGORY_HARDWARE)
    {
        g_signal_connect (icon, 
		          "map", 
		          G_CALLBACK (kiran_tray_notify_icon_map_callback), 
		          NULL);
    }

    if (type == ICON_SHOW_IN_PANEL)
    {
        gtk_box_pack_start (GTK_BOX (tray), GTK_WIDGET (icon), FALSE, TRUE, 0);
	gtk_widget_set_name (GTK_WIDGET (icon), "iconButtonPanel");
	kiran_tray_icons_refresh (tray);
    }
    else if (type == ICON_SHOW_IN_WINDOW)
    {
        kiran_notify_icon_window_add_icon (KIRAN_NOTIFY_ICON_WINDOW (priv->icons_win), icon); 
	gtk_widget_set_name (GTK_WIDGET (icon), "iconButton");

	if (!gtk_widget_is_visible (priv->icons_win_button))
            gtk_widget_show (priv->icons_win_button);
	position_notify_icon_window (tray, FALSE);
    }

    priv->icons = g_slist_prepend (priv->icons, icon);
}	

static void
kiran_tray_notify_icon_removed (KiranTrayManager *manager,
			        KiranNotifyIcon  *icon,
			        KiranTray        *tray)
{
    KiranTrayPrivate *priv;
    const char *id;
    gint type;

    g_return_if_fail (KIRAN_IS_TRAY_MANAGER (manager));
    g_return_if_fail (KIRAN_IS_NOTIFY_ICON (icon));
    g_return_if_fail (KIRAN_IS_TRAY (tray));

    priv = tray->priv;

    id = kiran_notify_icon_get_id (icon);
    type = kiran_tray_get_icon_type (priv->settings, id);

    if (type == ICON_SHOW_IN_PANEL)
    {
	gtk_container_remove (GTK_CONTAINER (tray), GTK_WIDGET (icon));
    }
    else if (type == ICON_SHOW_IN_WINDOW)
    {
        kiran_notify_icon_window_remove_icon (KIRAN_NOTIFY_ICON_WINDOW (priv->icons_win), icon); 

	if (kiran_notify_icon_window_get_icons_number (KIRAN_NOTIFY_ICON_WINDOW (priv->icons_win)) <= 0)
	{
	    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->icons_win_button), FALSE);
            gtk_widget_hide (priv->icons_win);
            gtk_widget_hide (priv->icons_win_button);
	}

	position_notify_icon_window (tray, FALSE);
    }

    priv->icons = g_slist_remove (priv->icons, icon);
}

static void
kiran_tray_add_manager (KiranTray *tray, KiranTrayManager *manager)
{
    KiranTrayPrivate *priv = tray->priv;

    priv->managers = g_slist_prepend (priv->managers, manager);

    g_signal_connect_object (manager, "icon-added",
		    	     G_CALLBACK (kiran_tray_notify_icon_added), tray, 0);
    g_signal_connect_object (manager, "icon-removed",
    		    	     G_CALLBACK (kiran_tray_notify_icon_removed), tray, 0);
}

GtkWidget *
kiran_tray_new (void)
{
    return g_object_new (KIRAN_TYPE_TRAY, NULL);
}
