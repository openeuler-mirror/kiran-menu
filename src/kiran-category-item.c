#include "kiran-category-item.h"

struct _KiranCategoryItem
{
    GtkEventBox parent;
    GtkWidget *label;

    gchar *name;
    gboolean clickable;
};

G_DEFINE_TYPE(KiranCategoryItem, kiran_category_item, GTK_TYPE_EVENT_BOX)

enum
{
    PROP_NAME = 1,
    PROP_CLICKABLE,
    PROP_MAX
};

enum {
    SIGNAL_CLICKED = 1,
    SIGNAL_MAX
};

static GParamSpec *pspecs[PROP_MAX] = {0};
static guint signals[SIGNAL_MAX] = {0};

void kiran_category_item_init(KiranCategoryItem *item)
{
    GtkWidget *label;
    GtkStyleContext *context;
    GValue value = G_VALUE_INIT;
    int min_width, min_height;
    
    context = gtk_widget_get_style_context(GTK_WIDGET(item));

    gtk_style_context_get_property(context, "min-width", GTK_STATE_FLAG_NORMAL, &value);
    min_width = g_value_get_int(&value);

    g_value_unset(&value);
    gtk_style_context_get_property(context, "min-height", GTK_STATE_FLAG_NORMAL, &value);
    min_height = g_value_get_int(&value);
    g_value_unset(&value);

    item->name = NULL;
    item->clickable = FALSE;

    item->label = gtk_label_new(NULL);

    gtk_widget_set_hexpand(item->label, TRUE);
    gtk_widget_set_halign(item->label, GTK_ALIGN_START);
    gtk_widget_set_size_request(GTK_WIDGET(item), min_width, min_height);
    gtk_label_set_ellipsize(GTK_LABEL(item->label), PANGO_ELLIPSIZE_END);
    gtk_container_add(GTK_CONTAINER(item), item->label);
}

void kiran_category_item_finalize(GObject *obj)
{
    KiranCategoryItem *self = KIRAN_CATEGORY_ITEM(obj);

    g_free(self->name);
    G_OBJECT_CLASS(kiran_category_item_parent_class)->finalize(obj);
}

gboolean kiran_category_item_enter_notify(GtkWidget *widget, GdkEventCrossing *event)
{
    KiranCategoryItem *self = KIRAN_CATEGORY_ITEM(widget);

    if (self->clickable)
    {
        gtk_widget_set_state_flags(widget, GTK_STATE_FLAG_PRELIGHT, FALSE);
        gtk_widget_queue_draw(widget);
    }

    return FALSE;
}

gboolean kiran_category_item_leave_notify(GtkWidget *widget, GdkEventCrossing *event)
{
    GtkStateFlags flags;

    flags = gtk_widget_get_state_flags(widget);
    gtk_widget_set_state_flags(widget, flags & ~GTK_STATE_FLAG_PRELIGHT, TRUE);
    gtk_widget_queue_draw(widget);

    return FALSE;
}

gboolean kiran_category_item_button_release(GtkWidget *widget, GdkEventButton *event)
{
    KiranCategoryItem *self = KIRAN_CATEGORY_ITEM(widget);

    if (!self->clickable)
        return FALSE;

    if (event->button != 1)
        return FALSE;
    
    g_message("emit category clicked signal\n");
    g_signal_emit_by_name(widget, "clicked");

    return FALSE;
}

gboolean kiran_category_item_draw(GtkWidget *widget, cairo_t *cr)
{
    GtkStyleContext *context;
    GtkAllocation allocation;

    context = gtk_widget_get_style_context(widget);
    gtk_widget_get_allocation(widget, &allocation);

    gtk_style_context_set_state(context, gtk_widget_get_state_flags(widget));

    gtk_render_background(context, cr, 0, 0, allocation.width, allocation.height);
    gtk_render_frame(context, cr, 0, 0, allocation.width, allocation.height);

    gtk_container_propagate_draw(GTK_CONTAINER(widget), gtk_bin_get_child(GTK_BIN(widget)), cr);
    return FALSE;
}

void kiran_category_item_set_property(GObject *obj, guint prop_id, const GValue *value, GParamSpec *pspec)
{
    KiranCategoryItem *item = KIRAN_CATEGORY_ITEM(obj);

    switch (prop_id)
    {
    case PROP_CLICKABLE:
        item->clickable = g_value_get_boolean(value);
        if (item->clickable) {
            g_message("item clickable, add events\n");
            gtk_widget_add_events(GTK_WIDGET(item),
                                  GDK_BUTTON_RELEASE_MASK | GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK);
        }
        else
        {
            g_message("item not clickable, reset events\n");
            gtk_widget_set_events(GTK_WIDGET(item), 0);
        }
        break;
    case PROP_NAME:
        if (item->name)
            g_free(item->name);
        item->name = g_value_dup_string(value);
        gtk_label_set_text(GTK_LABEL(item->label), item->name);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
        break;
    }
}

void kiran_category_item_get_property(GObject *obj, guint prop_id, GValue *value, GParamSpec *pspec)
{
    KiranCategoryItem *item = KIRAN_CATEGORY_ITEM(obj);

    switch (prop_id)
    {
    case PROP_CLICKABLE:
        g_value_set_boolean(value, item->clickable);
        break;
    case PROP_NAME:
        g_value_set_string(value, item->name);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
        break;
    }
}

void kiran_category_item_class_init(KiranCategoryItemClass *kclass)
{
    pspecs[PROP_NAME] = g_param_spec_string("name",
                                            "name", "category name", NULL, G_PARAM_READWRITE);
    pspecs[PROP_CLICKABLE] = g_param_spec_boolean("clickable",
                                                  "clickable", "whether it's clickable", FALSE, G_PARAM_READWRITE);

    signals[SIGNAL_CLICKED] = g_signal_new("clicked",
            G_TYPE_FROM_CLASS(kclass),
            G_SIGNAL_RUN_FIRST,
            0,
            NULL,
            NULL,
            NULL,
            G_TYPE_NONE,
            0);

    G_OBJECT_CLASS(kclass)->get_property = kiran_category_item_get_property;
    G_OBJECT_CLASS(kclass)->set_property = kiran_category_item_set_property;

    GTK_WIDGET_CLASS(kclass)->draw = kiran_category_item_draw;
    GTK_WIDGET_CLASS(kclass)->enter_notify_event = kiran_category_item_enter_notify;
    GTK_WIDGET_CLASS(kclass)->leave_notify_event = kiran_category_item_leave_notify;
    GTK_WIDGET_CLASS(kclass)->button_release_event = kiran_category_item_button_release;
    g_object_class_install_properties(G_OBJECT_CLASS(kclass), PROP_MAX, pspecs);

    gtk_widget_class_set_css_name(GTK_WIDGET_CLASS(kclass), "kiran-category-item");
}

KiranCategoryItem *kiran_category_item_new(const gchar *name, gboolean clickable)
{
    return g_object_new(KIRAN_TYPE_CATEGORY_ITEM, "name", name, "clickable", clickable, NULL);
}