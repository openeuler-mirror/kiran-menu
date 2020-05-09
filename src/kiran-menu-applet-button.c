#include "kiran-menu-applet-button.h"
#include "kiran-menu-window.h"

struct _KiranMenuAppletButton
{
	GtkToggleButton parent;

	KiranMenuWindow *menu_window;
	GdkPixbuf *icon;
	MatePanelApplet *applet;
	gboolean icon_size_fixed;
	guint icon_size;
};

G_DEFINE_TYPE(KiranMenuAppletButton, kiran_menu_applet_button,
			  GTK_TYPE_TOGGLE_BUTTON)

static GtkSizeRequestMode kiran_menu_applet_button_get_size_request_mode(
	GtkWidget *widget)
{
	KiranMenuAppletButton *button;

	button = KIRAN_MENU_APPLET_BUTTON(widget);
	g_assert(button->applet != NULL);

	switch (mate_panel_applet_get_orient(button->applet))
	{
	case MATE_PANEL_APPLET_ORIENT_DOWN:
	case MATE_PANEL_APPLET_ORIENT_UP:
		return GTK_SIZE_REQUEST_WIDTH_FOR_HEIGHT;
	default:
		return GTK_SIZE_REQUEST_HEIGHT_FOR_WIDTH;
	}
}

static void kiran_menu_applet_button_get_preferred_height(GtkWidget *widget,
														  int *min_height,
														  int *natural_height)
{
	KiranMenuAppletButton *button;

	button = KIRAN_MENU_APPLET_BUTTON(widget);
	g_assert(button->applet != NULL);

	*min_height = *natural_height = mate_panel_applet_get_size(button->applet);
}

static void kiran_menu_applet_button_get_preferred_width_for_height(GtkWidget *widget,
																	int height,
																	int *min_width,
																	int *natural_width)
{
	*natural_width = *min_width = height;
}

static void kiran_menu_applet_button_get_preferred_width(GtkWidget *widget,
														 int *min_width,
														 int *natural_width)
{
	KiranMenuAppletButton *button;

	button = KIRAN_MENU_APPLET_BUTTON(widget);
	g_assert(button->applet != NULL);

	*min_width = *natural_width = mate_panel_applet_get_size(button->applet);
}

static void kiran_menu_applet_button_get_preferred_height_for_width(GtkWidget *widget,
																	int width,
																	int *min_height,
																	int *natural_height)
{
	*natural_height = *min_height = width;
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
		button->icon_size = allocation->height - 8;
	}
	else
		button->icon_size = allocation->width - 8;

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

	button = KIRAN_MENU_APPLET_BUTTON(widget);
	context = gtk_widget_get_style_context(widget);

	real_pixbuf = gdk_pixbuf_scale_simple(button->icon, button->icon_size,
										  button->icon_size, GDK_INTERP_BILINEAR);

	g_assert(real_pixbuf != NULL);
	gtk_widget_get_allocation(GTK_WIDGET(button), &allocation);

	gtk_style_context_set_state(context, gtk_widget_get_state_flags(widget));
	gtk_render_background(context, cr, 0, 0, allocation.width, allocation.height);

	cairo_save(cr);
	gdk_cairo_set_source_pixbuf(cr, real_pixbuf,
								(allocation.width - button->icon_size) / 2.0,
								(allocation.height - button->icon_size) / 2.0);
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
		gtk_widget_show_all(window);
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

static void menu_window_active_callback(GtkWindow *window, GParamSpec *spec, GtkToggleButton *button)
{
	g_message("window active changed %d, visible %d\n",
			gtk_window_is_active(window),
			gtk_widget_is_visible(window));
	if (!gtk_window_is_active(window) && gtk_widget_is_visible(window))
		gtk_toggle_button_set_active(button, FALSE);
}

void kiran_menu_applet_button_init(KiranMenuAppletButton *self)
{
	GtkIconTheme *icon_theme;

	self->applet = NULL;
	self->icon_size_fixed = FALSE;
	self->icon_size = 16;

	icon_theme = gtk_icon_theme_get_default();

	self->icon = gtk_icon_theme_load_icon(
		icon_theme, "start-here", 96,
		GTK_ICON_LOOKUP_FORCE_SIZE | GTK_ICON_LOOKUP_FORCE_SVG, NULL);

	self->menu_window = kiran_menu_window_new(GTK_WIDGET(self));
	g_signal_connect(kiran_menu_window_get_window(self->menu_window), "notify::is-active", G_CALLBACK(menu_window_active_callback), self);
	//gtk_window_set_default_size(GTK_WINDOW(self->menu_window), 300, 600);
}

void kiran_menu_applet_button_finalize(GObject *obj)
{
	KiranMenuAppletButton *self = KIRAN_MENU_APPLET_BUTTON(obj);

	g_object_unref(self->icon);
	g_object_unref(self->menu_window);
}

void kiran_menu_applet_button_class_init(KiranMenuAppletButtonClass *kclass)
{
	G_OBJECT_CLASS(kclass)->finalize = kiran_menu_applet_button_finalize;
	GTK_WIDGET_CLASS(kclass)->get_request_mode =
		kiran_menu_applet_button_get_size_request_mode;
	GTK_WIDGET_CLASS(kclass)->get_preferred_height =
		kiran_menu_applet_button_get_preferred_height;
	GTK_WIDGET_CLASS(kclass)->get_preferred_width =
		kiran_menu_applet_button_get_preferred_width;
	GTK_WIDGET_CLASS(kclass)->get_preferred_height_for_width =
		kiran_menu_applet_button_get_preferred_height_for_width;
	GTK_WIDGET_CLASS(kclass)->get_preferred_width_for_height =
		kiran_menu_applet_button_get_preferred_width_for_height;
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
	return button;
}