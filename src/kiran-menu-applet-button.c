#include "kiran-menu-applet-button.h"
#include "kiran-menu-window.h"

#define BUTTON_MARGIN 6

struct _KiranMenuAppletButton
{
	GtkToggleButton parent;

	KiranMenuWindow *menu_window;
	GdkPixbuf *icon;
	MatePanelApplet *applet;
	gboolean icon_size_fixed;
	guint icon_size;

	GtkIconTheme *icon_theme;
};

G_DEFINE_TYPE(KiranMenuAppletButton, kiran_menu_applet_button,
			  GTK_TYPE_TOGGLE_BUTTON)

static void kiran_menu_applet_button_get_preferred_height(GtkWidget *widget,
														  int *min_height,
														  int *natural_height)
{
	KiranMenuAppletButton *button = KIRAN_MENU_APPLET_BUTTON(widget);;
	int size = mate_panel_applet_get_size(button->applet);
	MatePanelAppletOrient orient =mate_panel_applet_get_orient(button->applet);

	if (orient == MATE_PANEL_APPLET_ORIENT_DOWN || orient == MATE_PANEL_APPLET_ORIENT_UP)
		*min_height = *natural_height = size;
	else
		*min_height = *natural_height = size + 2 * BUTTON_MARGIN;
}

static void kiran_menu_applet_button_get_preferred_width(GtkWidget *widget,
														 int *min_width,
														 int *natural_width)
{
	KiranMenuAppletButton *button = KIRAN_MENU_APPLET_BUTTON(widget);;
	int size = mate_panel_applet_get_size(button->applet);
	MatePanelAppletOrient orient =mate_panel_applet_get_orient(button->applet);

	if (orient == MATE_PANEL_APPLET_ORIENT_DOWN || orient == MATE_PANEL_APPLET_ORIENT_UP)
		*min_width = *natural_width = size + 2 * BUTTON_MARGIN;
	else
		*min_width = *natural_width = size;
}

static void kiran_menu_applet_button_size_allocate(GtkWidget *widget,
												   GtkAllocation *allocation)
{
	KiranMenuAppletButton *button;
	MatePanelAppletOrient orient;

	button = KIRAN_MENU_APPLET_BUTTON(widget);
	g_assert(button->applet != NULL);

	orient = mate_panel_applet_get_orient(button->applet);
	if (orient == MATE_PANEL_APPLET_ORIENT_UP ||
		orient == MATE_PANEL_APPLET_ORIENT_DOWN)
	{
		button->icon_size = allocation->height - 2 * BUTTON_MARGIN;
	}
	else
		button->icon_size = allocation->width - 2 * BUTTON_MARGIN;

	g_printerr("icon size %d, allocation %d x %d\n", button->icon_size,
			   allocation->width, allocation->height);

	GTK_WIDGET_CLASS(kiran_menu_applet_button_parent_class)
		->size_allocate(widget, allocation);
}

static gboolean kiran_menu_applet_button_draw(GtkWidget *widget, cairo_t *cr)
{
	GdkPixbuf *real_pixbuf;
	KiranMenuAppletButton *button;
	GtkAllocation allocation;
	GtkStyleContext *context;
	int pixbuf_width, pixbuf_height;

	button = KIRAN_MENU_APPLET_BUTTON(widget);
	context = gtk_widget_get_style_context(widget);

	real_pixbuf = gdk_pixbuf_scale_simple(button->icon, button->icon_size,
										  button->icon_size, GDK_INTERP_BILINEAR);

	g_assert(real_pixbuf != NULL);

	pixbuf_width = gdk_pixbuf_get_width(real_pixbuf);
	pixbuf_height = gdk_pixbuf_get_height(real_pixbuf);
	gtk_widget_get_allocation(GTK_WIDGET(button), &allocation);

	gtk_style_context_set_state(context, gtk_widget_get_state_flags(widget));
	gtk_render_background(context, cr, 0, 0, allocation.width, allocation.height);

	cairo_save(cr);
	gdk_cairo_set_source_pixbuf(cr, real_pixbuf,
								(allocation.width - pixbuf_width) / 2.0,
								(allocation.height - pixbuf_height) / 2.0);
	cairo_paint(cr);
	cairo_restore(cr);

	g_object_unref(real_pixbuf);

	return FALSE;
}

static void kiran_menu_applet_button_toggled(GtkToggleButton *button)
{
	MatePanelAppletOrient orient;
	KiranMenuAppletButton *self = KIRAN_MENU_APPLET_BUTTON(button);
	int root_x, root_y;
	GtkAllocation allocation, popup_allocation;
	GtkWidget *window;

	window = kiran_menu_window_get_window(self->menu_window);
	orient = mate_panel_applet_get_orient(self->applet);
	if (gtk_toggle_button_get_active(button))
	{
		gtk_widget_show(window);

		//将开始菜单窗口总保持在上层
		gtk_window_set_keep_above(GTK_WINDOW(window), TRUE);
		//将开始菜单窗口视图重置回初始状态
		kiran_menu_window_reset_layout(self->menu_window);

		gdk_window_get_origin(gtk_widget_get_window(GTK_WIDGET(button)), &root_x,
							  &root_y);
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
	}
	else
	{
		gtk_widget_hide(window);
	}
}

void kiran_menu_applet_button_untoggle(KiranMenuAppletButton *self)
{
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(self), FALSE);
}

static void kiran_menu_applet_button_update(KiranMenuAppletButton *self)
{
	self->icon = gtk_icon_theme_load_icon(
		self->icon_theme, "start-here", 96,
		GTK_ICON_LOOKUP_FORCE_SIZE, NULL);

	gtk_widget_queue_draw(GTK_WIDGET(self));
}

void kiran_menu_applet_button_init(KiranMenuAppletButton *self)
{
	GtkIconTheme *icon_theme;

	self->applet = NULL;
	self->icon_size_fixed = FALSE;
	self->icon_size = 16;

	self->icon_theme = gtk_icon_theme_get_default();

	self->icon = gtk_icon_theme_load_icon(
		self->icon_theme, "start-here", 96,
		GTK_ICON_LOOKUP_FORCE_SIZE, NULL);

	self->menu_window = kiran_menu_window_new(GTK_WIDGET(self));

	/**
	 * 当窗口隐藏时更新插件按钮状态
	 */
	g_signal_connect_swapped(kiran_menu_window_get_window(self->menu_window), "hide", G_CALLBACK(kiran_menu_applet_button_untoggle), self);
	g_signal_connect_swapped(self->icon_theme, "changed", G_CALLBACK(kiran_menu_applet_button_update), self);
}

void kiran_menu_applet_button_finalize(GObject *obj)
{
	KiranMenuAppletButton *self = KIRAN_MENU_APPLET_BUTTON(obj);

	g_signal_handlers_disconnect_by_func(self->icon_theme, gtk_widget_queue_draw, self);

	g_object_unref(self->icon);
	g_object_unref(self->menu_window);

}

void kiran_menu_applet_button_class_init(KiranMenuAppletButtonClass *kclass)
{
	G_OBJECT_CLASS(kclass)->finalize = kiran_menu_applet_button_finalize;

	GTK_WIDGET_CLASS(kclass)->get_preferred_height =
		kiran_menu_applet_button_get_preferred_height;
	GTK_WIDGET_CLASS(kclass)->get_preferred_width =
		kiran_menu_applet_button_get_preferred_width;
	GTK_WIDGET_CLASS(kclass)->size_allocate =
		kiran_menu_applet_button_size_allocate;
	GTK_WIDGET_CLASS(kclass)->draw = kiran_menu_applet_button_draw;

	GTK_TOGGLE_BUTTON_CLASS(kclass)->toggled = kiran_menu_applet_button_toggled;
}

KiranMenuAppletButton *kiran_menu_applet_button_new(MatePanelApplet *applet)
{
	KiranMenuAppletButton *button;

	button = g_object_new(KIRAN_TYPE_MENU_APPLET_BUTTON, NULL);

	button->applet = applet;
	g_signal_connect_swapped(applet, "change-size", G_CALLBACK(gtk_widget_queue_resize), button);
	g_signal_connect_swapped(applet, "change-orient", G_CALLBACK(gtk_widget_queue_resize), button);

	return button;
}
