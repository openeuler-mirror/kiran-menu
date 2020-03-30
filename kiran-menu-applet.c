#include <mate-panel-applet.h>
#include <gtk/gtk.h>

static void
kiran_menu_popup(GtkToggleButton *button, gpointer userdata)
{
	GtkWidget *window;
	int root_x, root_y;
	MatePanelApplet *applet = userdata;
	GtkAllocation allocation, popup_allocation;
	MatePanelAppletOrient orient;

	orient = mate_panel_applet_get_orient(applet);
	window = g_object_get_data(G_OBJECT(button), "menu-window");

	if (gtk_toggle_button_get_active(button))
	{
		if (!window)
		{
			window = gtk_window_new(GTK_WINDOW_POPUP);

			gtk_window_set_default_size(GTK_WINDOW(window), 300, 600);
			g_object_set_data_full(G_OBJECT(button), "menu-window", window, g_object_unref);

		}
		
		gtk_widget_show_all(window);
		gdk_window_get_origin(gtk_widget_get_window(GTK_WIDGET(button)), &root_x, &root_y);
		gtk_widget_get_allocation(GTK_WIDGET(button), &allocation);
		gtk_widget_get_allocation(window, &popup_allocation);

		switch (orient)
		{
		case MATE_PANEL_APPLET_ORIENT_UP:
			root_y -= popup_allocation.height;
			break;
		case MATE_PANEL_APPLET_ORIENT_DOWN:
			root_y += allocation.height;
			break;
		case MATE_PANEL_APPLET_ORIENT_LEFT:
			root_x -= popup_allocation.width;
			break;
		case MATE_PANEL_APPLET_ORIENT_RIGHT:
			root_x += allocation.width;			
			break;
		default:
			g_error("invalid applet orientation: %d\n", orient);
			break;
		}
		gtk_window_move(GTK_WINDOW(window), root_x, root_y);
	} else {
		if (window)
			gtk_widget_hide(window);
	}
}

static gboolean
kiran_menu_applet_fill (MatePanelApplet *applet,
		   const gchar *iid,
		   gpointer     data)
{
	GtkWidget *button;
	GtkWidget *image;
	MatePanelAppletOrient orient;

	if (strcmp(iid, "KiranMenuApplet"))
		return FALSE;

	image = gtk_image_new_from_icon_name("start-here", GTK_ICON_SIZE_LARGE_TOOLBAR);
	button = gtk_toggle_button_new();
	orient = mate_panel_applet_get_orient(applet);

	g_signal_connect(button, "toggled", G_CALLBACK(kiran_menu_popup), applet);
	gtk_container_add(GTK_CONTAINER(button), GTK_WIDGET(image));
	gtk_container_add(GTK_CONTAINER(applet), button);
	gtk_widget_show_all(GTK_WIDGET(applet));
	return TRUE;
}

MATE_PANEL_APPLET_OUT_PROCESS_FACTORY("KiranMenuAppletFactory",
		PANEL_TYPE_APPLET,
		"kiran_menu_applet",
		kiran_menu_applet_fill,
		NULL);

