#include "kiran-search-entry.h"
#include <glib/gi18n.h>

struct _KiranSearchEntry
{
    GtkSearchEntry parent;

    gchar *primary_icon;
};

G_DEFINE_TYPE(KiranSearchEntry, kiran_search_entry, GTK_TYPE_SEARCH_ENTRY)

void kiran_search_entry_init(KiranSearchEntry *entry)
{
    entry->primary_icon = NULL;
}

void kiran_search_entry_finalize(GObject *obj)
{
    KiranSearchEntry *self = kiran_search_entry(obj);

    g_free(self->primary_icon);
    G_OBJECT_CLASS(kiran_search_entry_parent_class)->finalize(obj);
}

gboolean kiran_search_entry_draw(GtkWidget *widget, cairo_t *cr)
{
    GtkStyleContext *context;
    PangoLayout *pango_layout;
    GtkEntry *entry = GTK_ENTRY(widget);
    GdkRGBA placeholder_color;
    GtkStateFlags flags;

    flags = gtk_widget_get_state_flags(widget);

    GTK_WIDGET_CLASS(kiran_search_entry_parent_class)->draw(widget, cr);
    if (flags & GTK_STATE_FLAG_FOCUSED)
    {
        //当前已获取到焦点，这个时候gtk本身不会绘制placeholder文本
        const char *placeholder_text = gtk_entry_get_placeholder_text(entry);
        if (!gtk_entry_get_text_length(entry) && placeholder_text)
        {
            //编辑框内无内容
            int x, y;

            context = gtk_widget_get_style_context(widget);
            if (gtk_style_context_lookup_color(context, "placeholder_text_color", &placeholder_color))
            { //读取样式表中的placeholder文本颜色
                cairo_set_source_rgba(cr, placeholder_color.red,
                                      placeholder_color.green,
                                      placeholder_color.blue,
                                      1.0);
            }
            else
                g_critical("Failed to get placeholder color\n");

            pango_layout = gtk_entry_get_layout(GTK_ENTRY(widget));
            pango_layout_set_text(pango_layout, placeholder_text, -1);

            gtk_entry_get_layout_offsets(GTK_ENTRY(widget), &x, &y);
            cairo_move_to(cr, x, y);
            pango_cairo_show_layout(cr, pango_layout);
            pango_layout_set_text(pango_layout, "", -1);
        }
    }

    return FALSE;
}

void kiran_search_entry_class_init(KiranSearchEntryClass *kclass)
{
    GTK_WIDGET_CLASS(kclass)->draw = kiran_search_entry_draw;
    G_OBJECT_CLASS(kclass)->finalize = kiran_search_entry_finalize;
}

KiranSearchEntry *kiran_search_entry_new(void)
{
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_resource_at_scale("/kiran-menu/action/search",
                    16, 16, TRUE, NULL);

    return g_object_new(KIRAN_TYPE_SEARCH_ENTRY, 
                    "placeholder-text", _("Search for applications"),
                    "primary-icon-pixbuf", pixbuf,
                    NULL);
}