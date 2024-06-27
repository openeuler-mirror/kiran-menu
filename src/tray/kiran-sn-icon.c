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

#include "kiran-sn-icon.h"
#include <glib/gstdio.h>
#include <math.h>
#include "kiran-notify-icon.h"
#include "kiran-sn-icon-menu.h"
#include "kiran-sn-item-gen.h"

#define KIRAN_SN_ICON_INTERFACE "org.kde.StatusNotifierItem"

typedef struct
{
    cairo_surface_t *surface;
    guint width;
    guint height;
} KiranSnIconPixmap;

typedef struct
{
    gchar *icon_name;
    KiranSnIconPixmap **icon_pixmap;
    gchar *title;
    gchar *text;
} KiranSnTooltip;

struct _KiranSnIconPrivate
{
    GtkWidget *image;
    GtkOrientation orientation;
    gint icon_size;
    gint effective_icon_size;
    gchar *icon_name;
    KiranSnIconPixmap **icon_pixmap;
    GCancellable *cancellable;
    KiranSnItemGen *proxy;
    gchar *bus_name;
    gchar *object_path;
    gchar *id;
    KiranSnTooltip *tooltip;
    gchar *menu;
    gboolean item_is_menu;
    GtkMenu *gmenu;
    gint update_id;
    gchar *name;
    KiranNotifyIconCategory category;
    gchar *app_category;
    gchar *icon;
};

enum
{
    PROP_0,
    PROP_BUS_NAME,
    PROP_OBJECT_PATH,
    PROP_ORIENTATION,
    PROP_ICON_SIZE,
    LAST_PROP
};

enum
{
    SIGNAL_READY,
    LAST_SIGNAL
};

#define KIRAN_SN_ITEM_INTERFACE "org.kde.StatusNotifierItem"

static guint signals[LAST_SIGNAL] = {0};
static void kiran_notify_icon_init(KiranNotifyIconInterface *iface);
static const gchar *kiran_sn_icon_get_id(KiranNotifyIcon *icon);
static KiranNotifyIconCategory kiran_sn_icon_get_category(KiranNotifyIcon *icon);
static const gchar *kiran_sn_icon_get_app_category(KiranNotifyIcon *icon);
static KiranNotifyIconWay kiran_sn_icon_get_way(KiranNotifyIcon *icon);
static const gchar *kiran_sn_icon_get_name(KiranNotifyIcon *icon);
static const gchar *kiran_sn_icon_get_icon(KiranNotifyIcon *icon);
static KiranSnIconPixmap **icon_pixmap_new(GVariant *variant);
static void icon_pixmap_free(KiranSnIconPixmap **data);

#define KIRAN_SN_ICON_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), KIRAN_TYPE_SN_ICON, KiranSnIconPrivate))

G_DEFINE_TYPE_WITH_CODE(KiranSnIcon, kiran_sn_icon, GTK_TYPE_BUTTON,
                        G_IMPLEMENT_INTERFACE(GTK_TYPE_ORIENTABLE, NULL)
                            G_IMPLEMENT_INTERFACE(KIRAN_TYPE_NOTIFY_ICON, kiran_notify_icon_init))

static void
kiran_notify_icon_init(KiranNotifyIconInterface *iface)
{
    iface->get_id = kiran_sn_icon_get_id;
    iface->get_category = kiran_sn_icon_get_category;
    iface->get_app_category = kiran_sn_icon_get_app_category;
    iface->get_name = kiran_sn_icon_get_name;
    iface->get_icon = kiran_sn_icon_get_icon;
    iface->get_way = kiran_sn_icon_get_way;
}

static const gchar *
kiran_sn_icon_get_id(KiranNotifyIcon *icon)
{
    KiranSnIconPrivate *priv = KIRAN_SN_ICON(icon)->priv;

    return priv->id;
}

static KiranNotifyIconCategory
kiran_sn_icon_get_category(KiranNotifyIcon *icon)
{
    KiranSnIconPrivate *priv = KIRAN_SN_ICON(icon)->priv;

    return priv->category;
}

static const gchar *
kiran_sn_icon_get_app_category(KiranNotifyIcon *icon)
{
    KiranSnIconPrivate *priv = KIRAN_SN_ICON(icon)->priv;

    return priv->app_category;
}

static KiranNotifyIconWay
kiran_sn_icon_get_way(KiranNotifyIcon *icon)
{
    return KIRAN_NOTIFY_ICON_WAY_DBUS;
}

static const gchar *
kiran_sn_icon_get_name(KiranNotifyIcon *icon)
{
    KiranSnIconPrivate *priv = KIRAN_SN_ICON(icon)->priv;

    return priv->name;
}

static const gchar *
kiran_sn_icon_get_icon(KiranNotifyIcon *icon)
{
    KiranSnIconPrivate *priv = KIRAN_SN_ICON(icon)->priv;

    return priv->icon;
}

static void
kiran_sn_tooltip_free(KiranSnTooltip *tooltip)
{
    if (tooltip == NULL)
        return;

    g_free(tooltip->icon_name);
    icon_pixmap_free(tooltip->icon_pixmap);
    g_free(tooltip->title);
    g_free(tooltip->text);

    g_free(tooltip);
}

static KiranSnTooltip *
kiran_sn_tooltip_new(GVariant *variant)
{
    const gchar *icon_name;
    GVariant *icon_pixmap;
    const gchar *title;
    const gchar *text;
    KiranSnTooltip *tooltip;

    if (variant == NULL)
        return NULL;

    if (!g_variant_is_of_type(variant, G_VARIANT_TYPE("(sa(iiay)ss)")))
    {
        g_warning("Type for 'ToolTip' property should be '(sa(iiay)ss)' "
                  "but got '%s'",
                  g_variant_get_type_string(variant));

        return NULL;
    }

    g_variant_get(variant, "(&s@a(iiay)&s&s)",
                  &icon_name, &icon_pixmap,
                  &title, &text);

    tooltip = g_new0(KiranSnTooltip, 1);

    tooltip->icon_name = g_strdup(icon_name);
    tooltip->icon_pixmap = icon_pixmap_new(icon_pixmap);
    tooltip->title = g_strdup(title);
    tooltip->text = g_strdup(text);

    g_variant_unref(icon_pixmap);
    return tooltip;
}

static cairo_surface_t *
get_icon_by_name(const gchar *icon_name,
                 gint requested_size,
                 gint scale)
{
    GtkIconTheme *icon_theme;
    gint *sizes;
    gint i;
    gint chosen_size = 0;

    g_return_val_if_fail(icon_name != NULL && icon_name[0] != '\0', NULL);
    g_return_val_if_fail(requested_size > 0, NULL);

    icon_theme = gtk_icon_theme_get_default();
    gtk_icon_theme_rescan_if_needed(icon_theme);

    sizes = gtk_icon_theme_get_icon_sizes(icon_theme, icon_name);
    for (i = 0; sizes[i] != 0; i++)
    {
        if (sizes[i] == requested_size ||
            sizes[i] == -1) /* scalable */
        {
            /* perfect match, stop here */
            chosen_size = requested_size;
            break;
        }
        else if (sizes[i] < requested_size && sizes[i] > chosen_size)
            chosen_size = sizes[i];
    }
    g_free(sizes);

    if (chosen_size == 0)
        chosen_size = requested_size;

    return gtk_icon_theme_load_surface(icon_theme, icon_name,
                                       chosen_size, scale,
                                       NULL, GTK_ICON_LOOKUP_FORCE_SIZE, NULL);
}

static cairo_surface_t *
surface_from_variant(GVariant *variant,
                     gint width,
                     gint height)
{
    cairo_format_t format;
    gint stride;
    guint32 *data;
    gint x;
    gint y;
    guchar *p;

    format = CAIRO_FORMAT_ARGB32;
    stride = cairo_format_stride_for_width(format, width);
    data = (guint32 *)g_variant_get_data(variant);

#if G_BYTE_ORDER == G_LITTLE_ENDIAN
    {
        gint i;

        for (i = 0; i < width * height; i++)
            data[i] = GUINT32_FROM_BE(data[i]);
    }
#endif

    p = (guchar *)data;
    /* premultiply alpha for Cairo */
    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            guchar alpha = p[x * 4 + 3];

            p[x * 4 + 0] = (p[x * 4 + 0] * alpha) / 255;
            p[x * 4 + 1] = (p[x * 4 + 1] * alpha) / 255;
            p[x * 4 + 2] = (p[x * 4 + 2] * alpha) / 255;
        }

        p += stride;
    }

    return cairo_image_surface_create_for_data((guchar *)data, format,
                                               width, height, stride);
}

static cairo_surface_t *
icon_surface_new(GVariant *variant,
                 gint width,
                 gint height)
{
    cairo_surface_t *surface;
    cairo_surface_t *tmp;
    cairo_t *cr;

    surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    if (cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS)
        return NULL;

    tmp = surface_from_variant(variant, width, height);
    if (cairo_surface_status(tmp) != CAIRO_STATUS_SUCCESS)
    {
        cairo_surface_destroy(surface);
        return NULL;
    }

    cr = cairo_create(surface);
    if (cairo_status(cr) != CAIRO_STATUS_SUCCESS)
    {
        cairo_surface_destroy(surface);
        cairo_surface_destroy(tmp);
        return NULL;
    }

    cairo_set_source_surface(cr, tmp, 0, 0);
    cairo_paint(cr);

    cairo_surface_destroy(tmp);
    cairo_destroy(cr);

    return surface;
}

static KiranSnIconPixmap **
icon_pixmap_new(GVariant *variant)
{
    GPtrArray *array;
    GVariantIter iter;
    gint width;
    gint height;
    GVariant *value;

    if (variant == NULL ||
        g_variant_iter_init(&iter, variant) == 0)
        return NULL;

    array = g_ptr_array_new();
    while (g_variant_iter_next(&iter, "(ii@ay)", &width, &height, &value))
    {
        cairo_surface_t *surface;

        if (width == 0 || height == 0)
        {
            g_variant_unref(value);
            continue;
        }

        surface = icon_surface_new(value, width, height);
        g_variant_unref(value);

        if (surface != NULL)
        {
            KiranSnIconPixmap *pixmap;

            pixmap = g_new0(KiranSnIconPixmap, 1);

            pixmap->surface = surface;
            pixmap->width = width;
            pixmap->height = height;

            g_ptr_array_add(array, pixmap);
        }
    }

    g_ptr_array_add(array, NULL);

    return (KiranSnIconPixmap **)g_ptr_array_free(array, FALSE);
}

static void
icon_pixmap_free(KiranSnIconPixmap **data)
{
    gint i;

    if (data == NULL)
        return;

    for (i = 0; data[i] != NULL; i++)
    {
        cairo_surface_destroy(data[i]->surface);
        g_free(data[i]);
    }

    g_free(data);
}

static cairo_surface_t *
scale_surface(KiranSnIconPixmap *pixmap,
              GtkOrientation orientation,
              gint size)
{
    gdouble ratio;
    gdouble new_width;
    gdouble new_height;
    gdouble scale_x;
    gdouble scale_y;
    gint width;
    gint height;
    cairo_content_t content;
    cairo_surface_t *scaled;
    cairo_t *cr;

    g_return_val_if_fail(pixmap != NULL, NULL);

    ratio = pixmap->width / (gdouble)pixmap->height;
    if (orientation == GTK_ORIENTATION_HORIZONTAL)
    {
        new_height = (gdouble)size;
        new_width = new_height * ratio;
    }
    else
    {
        new_width = (gdouble)size;
        new_height = new_width * ratio;
    }

    scale_x = new_width / pixmap->width;
    scale_y = new_height / pixmap->height;

    width = ceil(new_width);
    height = ceil(new_height);

    content = CAIRO_CONTENT_COLOR_ALPHA;
    scaled = cairo_surface_create_similar(pixmap->surface, content, width, height);
    cr = cairo_create(scaled);

    cairo_scale(cr, scale_x, scale_y);
    cairo_set_source_surface(cr, pixmap->surface, 0, 0);
    cairo_paint(cr);

    cairo_destroy(cr);
    return scaled;
}

static gint
compare_size(gconstpointer a,
             gconstpointer b,
             gpointer user_data)
{
    KiranSnIconPixmap *p1;
    KiranSnIconPixmap *p2;
    GtkOrientation orientation;

    p1 = (KiranSnIconPixmap *)a;
    p2 = (KiranSnIconPixmap *)b;
    orientation = GPOINTER_TO_UINT(user_data);

    if (orientation == GTK_ORIENTATION_HORIZONTAL)
        return p1->height - p2->height;
    else
        return p1->width - p2->width;
}

static cairo_surface_t *
get_surface(KiranSnIcon *icon,
            GtkOrientation orientation,
            gint size)
{
    KiranSnIconPrivate *priv;
    gint i;
    GList *pixmaps = NULL;
    KiranSnIconPixmap *pixmap = NULL;
    GList *l;

    priv = KIRAN_SN_ICON_GET_PRIVATE(icon);

    g_assert(priv->icon_pixmap != NULL && priv->icon_pixmap[0] != NULL);

    for (i = 0; priv->icon_pixmap[i] != NULL; i++)
        pixmaps = g_list_prepend(pixmaps, priv->icon_pixmap[i]);

    pixmaps = g_list_sort_with_data(pixmaps, compare_size,
                                    GUINT_TO_POINTER(orientation));

    pixmap = (KiranSnIconPixmap *)pixmaps->data;
    for (l = pixmaps; l != NULL; l = l->next)
    {
        KiranSnIconPixmap *p = (KiranSnIconPixmap *)l->data;

        if (p->height > size && p->width > size)
        {
            break;
        }
        pixmap = p;
    }

    g_list_free(pixmaps);

    if (pixmap == NULL || pixmap->surface == NULL)
        return NULL;
    else if (pixmap->height > size || pixmap->width > size)
        return scale_surface(pixmap, orientation, size);
    else
        return cairo_surface_reference(pixmap->surface);
}

static void
update(KiranSnIcon *icon)
{
    KiranSnIconPrivate *priv;
    KiranSnTooltip *tip;
    gint icon_size;

    priv = KIRAN_SN_ICON_GET_PRIVATE(icon);

    if (priv->icon_size > 0)
        icon_size = priv->icon_size;
    else
        icon_size = MAX(1, priv->effective_icon_size);

    if (priv->icon_name != NULL && priv->icon_name[0] != '\0')
    {
        cairo_surface_t *surface;
        gint scale;

        scale = gtk_widget_get_scale_factor(GTK_WIDGET(priv->image));
        surface = get_icon_by_name(priv->icon_name, icon_size, scale);
        if (!surface)
        {
            GdkPixbuf *pixbuf;

            /*try to find icons specified by path and filename*/
            pixbuf = gdk_pixbuf_new_from_file(priv->icon_name, NULL);
            if (pixbuf && icon_size > 1)
            {
                /*An icon specified by path and filename may be the wrong size for the tray */
                pixbuf = gdk_pixbuf_scale_simple(pixbuf, icon_size - 2, icon_size - 2, GDK_INTERP_BILINEAR);
                surface = gdk_cairo_surface_create_from_pixbuf(pixbuf, scale, NULL);
            }

            if (pixbuf)
            {
                g_object_unref(pixbuf);
            }
        }
        if (!surface)
        {
            /*deal with missing icon or failure to load icon*/
            surface = get_icon_by_name("image-missing", icon_size, scale);
        }

        gtk_image_set_from_surface(GTK_IMAGE(priv->image), surface);
        if(surface)
        {
            cairo_surface_destroy(surface);
        }
        g_free(priv->icon);
        priv->icon = g_strdup(priv->icon_name);
    }
    else if (priv->icon_pixmap != NULL && priv->icon_pixmap[0] != NULL)
    {
        cairo_surface_t *surface;

        surface = get_surface(icon,
                              gtk_orientable_get_orientation(GTK_ORIENTABLE(icon)),
                              icon_size);
        if (surface != NULL)
        {
            gtk_image_set_from_surface(GTK_IMAGE(priv->image), surface);

            g_free(priv->icon);
            priv->icon = g_strdup_printf("%s/.config/kiran-tray/icon/%s.png",
                                         g_get_home_dir(),
                                         priv->id);

            if (!g_file_test(priv->icon, G_FILE_TEST_EXISTS))
            {
                gchar *dir = NULL;

                dir = g_strdup_printf("%s/.config/kiran-tray/icon", g_get_home_dir());
                if (dir)
                {
                    if (!g_file_test(dir, G_FILE_TEST_EXISTS))
                    {
                        g_mkdir_with_parents(dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
                    }
                    free(dir);
                }

                cairo_surface_write_to_png(surface, priv->icon);
            }

            cairo_surface_destroy(surface);
        }
    }
    else
    {
        gtk_image_set_from_icon_name(GTK_IMAGE(priv->image), "image-missing", GTK_ICON_SIZE_MENU);
        gtk_image_set_pixel_size(GTK_IMAGE(priv->image), icon_size);
    }
}

static gboolean
update_cb(gpointer user_data)
{
    KiranSnIcon *icon;
    KiranSnIconPrivate *priv;

    icon = KIRAN_SN_ICON(user_data);
    priv = icon->priv;

    priv->update_id = 0;
    update(icon);

    return G_SOURCE_REMOVE;
}

static void
queue_update(KiranSnIcon *icon)
{
    KiranSnIconPrivate *priv = icon->priv;

    if (priv->update_id != 0)
        return;

    priv->update_id = g_timeout_add(10, update_cb, icon);
    g_source_set_name_by_id(priv->update_id, "[status-notifier] update_cb");
}

static GVariant *
get_property(GObject *source_object,
             GAsyncResult *res,
             gpointer user_data,
             gboolean *cancelled)
{
    GVariant *variant;
    GError *error;
    GVariant *property;

    error = NULL;
    variant = g_dbus_connection_call_finish(G_DBUS_CONNECTION(source_object),
                                            res, &error);

    *cancelled = FALSE;
    if (g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
    {
        *cancelled = TRUE;
        g_error_free(error);
        return NULL;
    }

    if (g_error_matches(error, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS))
    {
        g_error_free(error);
        return NULL;
    }

    if (error)
    {
        g_warning("%s", error->message);
        g_error_free(error);
        return NULL;
    }

    g_variant_get(variant, "(v)", &property);
    g_variant_unref(variant);

    return property;
}

static void
update_property(KiranSnIcon *icon,
                const gchar *property,
                GAsyncReadyCallback callback)
{
    KiranSnIconPrivate *priv = icon->priv;
    GDBusProxy *proxy;

    proxy = G_DBUS_PROXY(priv->proxy);

    g_dbus_connection_call(g_dbus_proxy_get_connection(proxy),
                           kiran_sn_icon_get_bus_name(icon),
                           kiran_sn_icon_get_object_path(icon),
                           "org.freedesktop.DBus.Properties", "Get",
                           g_variant_new("(ss)", KIRAN_SN_ICON_INTERFACE, property),
                           G_VARIANT_TYPE("(v)"),
                           G_DBUS_CALL_FLAGS_NONE, -1,
                           priv->cancellable, callback, icon);
}

static void
update_icon_name(GObject *source_object,
                 GAsyncResult *res,
                 gpointer user_data)
{
    KiranSnIcon *icon;
    KiranSnIconPrivate *priv;
    GVariant *variant;
    gboolean cancelled;

    variant = get_property(source_object, res, user_data, &cancelled);
    if (cancelled)
        return;

    icon = KIRAN_SN_ICON(user_data);
    priv = icon->priv;
    g_clear_pointer(&priv->icon_name, g_free);
    priv->icon_name = g_variant_dup_string(variant, NULL);
    g_clear_pointer(&variant, g_variant_unref);

    queue_update(icon);
}

static void
update_icon_pixmap(GObject *source_object,
                   GAsyncResult *res,
                   gpointer user_data)
{
    KiranSnIcon *icon;
    KiranSnIconPrivate *priv;
    GVariant *variant;
    gboolean cancelled;

    variant = get_property(source_object, res, user_data, &cancelled);
    if (cancelled)
        return;

    icon = KIRAN_SN_ICON(user_data);
    priv = icon->priv;

    g_clear_pointer(&priv->icon_pixmap, icon_pixmap_free);
    priv->icon_pixmap = icon_pixmap_new(variant);
    g_clear_pointer(&variant, g_variant_unref);

    queue_update(icon);
}

static void
new_icon_cb(KiranSnIcon *icon)
{
    update_property(icon, "IconName", update_icon_name);
    update_property(icon, "IconPixmap", update_icon_pixmap);
}

static void
update_tooltip(GObject *source_object,
               GAsyncResult *res,
               gpointer user_data)
{
    KiranSnIcon *icon;
    KiranSnIconPrivate *priv;
    GVariant *variant;
    gboolean cancelled;

    variant = get_property(source_object, res, user_data, &cancelled);
    if (cancelled)
        return;

    icon = KIRAN_SN_ICON(user_data);
    priv = icon->priv;

    g_clear_pointer(&priv->tooltip, kiran_sn_tooltip_free);
    priv->tooltip = kiran_sn_tooltip_new(variant);
    g_clear_pointer(&variant, g_variant_unref);

    queue_update(icon);
}

static void
new_tooltip_cb(KiranSnIcon *icon)
{
    update_property(icon, "ToolTip", update_tooltip);
}

static void
g_signal_cb(GDBusProxy *proxy,
            gchar *sender_name,
            gchar *signal_name,
            GVariant *parameters,
            KiranSnIcon *icon)
{
    if (g_strcmp0(signal_name, "NewIcon") == 0)
        new_icon_cb(icon);
    else if (g_strcmp0(signal_name, "NewToolTip") == 0)
        new_tooltip_cb(icon);
    else
        g_debug("signal '%s' not handled!", signal_name);
}

static void
get_all_cb(GObject *source_object,
           GAsyncResult *res,
           gpointer user_data)
{
    KiranSnIcon *icon;
    KiranSnIconPrivate *priv;
    GVariant *properties;
    GError *error;
    GVariantIter *iter;
    gchar *key;
    GVariant *value;

    error = NULL;
    properties = g_dbus_connection_call_finish(G_DBUS_CONNECTION(source_object),
                                               res,
                                               &error);

    if (g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
    {
        g_error_free(error);
        return;
    }

    if (error)
    {
        g_warning("%s", error->message);
        g_error_free(error);
        return;
    }

    icon = KIRAN_SN_ICON(user_data);
    priv = KIRAN_SN_ICON_GET_PRIVATE(icon);

    g_variant_get(properties, "(a{sv})", &iter);
    while (g_variant_iter_next(iter, "{sv}", &key, &value))
    {
        if (g_strcmp0(key, "IconName") == 0)
            priv->icon_name = g_variant_dup_string(value, NULL);
        else if (g_strcmp0(key, "IconPixmap") == 0)
            priv->icon_pixmap = icon_pixmap_new(value);
        else if (g_strcmp0(key, "Id") == 0)
        {
            gint i;

            priv->id = g_variant_dup_string(value, NULL);

            for (i = 0; i < G_N_ELEMENTS(wmclass_categories); i++)
            {
                if (g_strcmp0(priv->id, wmclass_categories[i].wm_class) == 0)
                {
                    g_free(priv->id);
                    priv->id = g_strdup(wmclass_categories[i].id);
                    priv->category = wmclass_categories[i].category;
                    break;
                }
            }
        }
        else if (g_strcmp0(key, "ToolTip") == 0)
            priv->tooltip = kiran_sn_tooltip_new(value);
        else if (g_strcmp0(key, "ToolTip") == 0)
            priv->tooltip = kiran_sn_tooltip_new(value);
        else if (g_strcmp0(key, "Menu") == 0)
            priv->menu = g_variant_dup_string(value, NULL);
        else if (g_strcmp0(key, "ItemIsMenu") == 0)
            priv->item_is_menu = g_variant_get_boolean(value);
        else if (g_strcmp0(key, "Title") == 0)
            priv->name = g_variant_dup_string(value, NULL);
        else if (g_strcmp0(key, "Category") == 0)
            priv->app_category = g_variant_dup_string(value, NULL);

        g_variant_unref(value);
        g_free(key);
    }

    g_variant_iter_free(iter);
    g_variant_unref(properties);

    if (!(priv->menu == NULL ||
          *(priv->menu) == '\0' ||
          g_strcmp0(priv->menu, "/") == 0))
    {
        priv->gmenu = kiran_sn_icon_menu_new(priv->bus_name, priv->menu);
        g_object_ref_sink(priv->gmenu);
    }

    g_signal_connect(priv->proxy, "g-signal",
                     G_CALLBACK(g_signal_cb), icon);

    update(icon);

    g_signal_emit(icon, signals[SIGNAL_READY], 0);
}

static void
proxy_ready_cb(GObject *source_object,
               GAsyncResult *res,
               gpointer user_data)
{
    KiranSnIcon *icon;
    KiranSnIconPrivate *priv;
    KiranSnItemGen *proxy;
    GError *error;

    icon = KIRAN_SN_ICON(user_data);
    priv = icon->priv;

    error = NULL;
    proxy = kiran_sn_item_gen_proxy_new_for_bus_finish(res, &error);

    if (g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
    {
        g_error_free(error);
        return;
    }

    priv->proxy = proxy;

    if (error)
    {
        g_warning("%s", error->message);
        g_error_free(error);
        return;
    }

    g_dbus_connection_call(g_dbus_proxy_get_connection(G_DBUS_PROXY(proxy)),
                           priv->bus_name,
                           priv->object_path,
                           "org.freedesktop.DBus.Properties", "GetAll",
                           g_variant_new("(s)", KIRAN_SN_ITEM_INTERFACE),
                           G_VARIANT_TYPE("(a{sv})"),
                           G_DBUS_CALL_FLAGS_NONE, -1,
                           priv->cancellable, get_all_cb, icon);
}

static void
kiran_sn_icon_constructed(GObject *object)
{
    KiranSnIcon *icon;
    KiranSnIconPrivate *priv;

    G_OBJECT_CLASS(kiran_sn_icon_parent_class)->constructed(object);

    icon = KIRAN_SN_ICON(object);
    priv = KIRAN_SN_ICON_GET_PRIVATE(icon);

    priv->cancellable = g_cancellable_new();
    kiran_sn_item_gen_proxy_new_for_bus(G_BUS_TYPE_SESSION,
                                        G_DBUS_PROXY_FLAGS_NONE,
                                        priv->bus_name,
                                        priv->object_path,
                                        priv->cancellable,
                                        proxy_ready_cb,
                                        icon);
}

static void
kiran_sn_icon_get_property(GObject *object,
                           guint property_id,
                           GValue *value,
                           GParamSpec *pspec)
{
    KiranSnIcon *icon;
    KiranSnIconPrivate *priv;

    icon = KIRAN_SN_ICON(object);
    priv = KIRAN_SN_ICON_GET_PRIVATE(icon);

    switch (property_id)
    {
    case PROP_BUS_NAME:
        g_value_set_string(value, priv->bus_name);
        break;

    case PROP_OBJECT_PATH:
        g_value_set_string(value, priv->object_path);
        break;

    case PROP_ORIENTATION:
        g_value_set_enum(value, priv->orientation);
        break;

    case PROP_ICON_SIZE:
        g_value_set_int(value, priv->icon_size);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void
kiran_sn_icon_set_property(GObject *object,
                           guint property_id,
                           const GValue *value,
                           GParamSpec *pspec)
{
    KiranSnIcon *icon;
    KiranSnIconPrivate *priv;

    icon = KIRAN_SN_ICON(object);
    priv = KIRAN_SN_ICON_GET_PRIVATE(icon);

    switch (property_id)
    {
    case PROP_BUS_NAME:
        priv->bus_name = g_value_dup_string(value);
        break;

    case PROP_OBJECT_PATH:
        priv->object_path = g_value_dup_string(value);
        break;

    case PROP_ORIENTATION:
        priv->orientation = g_value_get_enum(value);
        break;

    case PROP_ICON_SIZE:
        priv->icon_size = g_value_get_int(value);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void
kiran_sn_icon_dispose(GObject *object)
{
    KiranSnIcon *icon;
    KiranSnIconPrivate *priv;

    icon = KIRAN_SN_ICON(object);
    priv = KIRAN_SN_ICON_GET_PRIVATE(icon);

    if (priv->update_id != 0)
    {
        g_source_remove(priv->update_id);
        priv->update_id = 0;
    }

    g_cancellable_cancel(priv->cancellable);
    g_clear_object(&priv->cancellable);
    g_clear_object(&priv->proxy);
    g_clear_object(&priv->gmenu);

    G_OBJECT_CLASS(kiran_sn_icon_parent_class)->dispose(object);
}

static void
kiran_sn_icon_finalize(GObject *object)
{
    KiranSnIcon *icon;
    KiranSnIconPrivate *priv;

    icon = KIRAN_SN_ICON(object);
    priv = KIRAN_SN_ICON_GET_PRIVATE(icon);

    g_clear_pointer(&priv->bus_name, g_free);
    g_clear_pointer(&priv->object_path, g_free);
    g_clear_pointer(&priv->icon_pixmap, icon_pixmap_free);
    g_clear_pointer(&priv->id, g_free);
    g_clear_pointer(&priv->icon_name, g_free);
    g_clear_pointer(&priv->icon_pixmap, icon_pixmap_free);
    g_clear_pointer(&priv->tooltip, kiran_sn_tooltip_free);
    g_clear_pointer(&priv->menu, g_free);
    g_clear_pointer(&priv->name, g_free);
    g_clear_pointer(&priv->app_category, g_free);
    g_clear_pointer(&priv->icon, g_free);

    G_OBJECT_CLASS(kiran_sn_icon_parent_class)->finalize(object);
}

static void
install_properites(GObjectClass *gobject_class)
{
    g_object_class_install_property(gobject_class,
                                    PROP_BUS_NAME,
                                    g_param_spec_string("bus-name",
                                                        "bus-name",
                                                        "bus-name",
                                                        NULL,
                                                        G_PARAM_CONSTRUCT_ONLY |
                                                            G_PARAM_READWRITE |
                                                            G_PARAM_STATIC_STRINGS));

    g_object_class_install_property(gobject_class,
                                    PROP_OBJECT_PATH,
                                    g_param_spec_string("object-path",
                                                        "object-path",
                                                        "object-path",
                                                        NULL,
                                                        G_PARAM_CONSTRUCT_ONLY |
                                                            G_PARAM_READWRITE |
                                                            G_PARAM_STATIC_STRINGS));
    g_object_class_install_property(gobject_class,
                                    PROP_ORIENTATION,
                                    g_param_spec_enum("orientation",
                                                      "orientation",
                                                      "orientation",
                                                      GTK_TYPE_ORIENTATION,
                                                      GTK_ORIENTATION_HORIZONTAL,
                                                      G_PARAM_READWRITE));

    g_object_class_install_property(gobject_class,
                                    PROP_ICON_SIZE,
                                    g_param_spec_int("icon-size",
                                                     "icon-size",
                                                     "icon-size",
                                                     0,
                                                     G_MAXINT,
                                                     0,
                                                     G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
}

static void
context_menu_cb(GObject *source_object,
                GAsyncResult *res,
                gpointer user_data)
{
    KiranSnIcon *icon;
    KiranSnIconPrivate *priv;

    icon = KIRAN_SN_ICON(user_data);
    priv = KIRAN_SN_ICON_GET_PRIVATE(icon);

    kiran_sn_item_gen_call_context_menu_finish(priv->proxy,
                                               res,
                                               NULL);
}

static void
kiran_sn_icon_context_menu(KiranSnIcon *icon,
                           gint x,
                           gint y)
{
    KiranSnIconPrivate *priv;

    priv = KIRAN_SN_ICON_GET_PRIVATE(icon);

    kiran_sn_item_gen_call_context_menu(priv->proxy,
                                        x,
                                        y,
                                        NULL,
                                        context_menu_cb,
                                        icon);
}

static void
kiran_sn_icon_get_action_coordinates(KiranSnIcon *icon,
                                     gint *x,
                                     gint *y)
{
    GtkWidget *widget;
    KiranSnIconPrivate *priv;
    GdkWindow *window;
    GtkWidget *toplevel;
    gint width;
    gint height;

    priv = KIRAN_SN_ICON_GET_PRIVATE(icon);
    widget = GTK_WIDGET(icon);
    window = gtk_widget_get_window(widget);
    toplevel = gtk_widget_get_toplevel(widget);

    gdk_window_get_geometry(window, x, y, &width, &height);
    gtk_widget_translate_coordinates(widget, toplevel, *x, *y, x, y);

    if (priv->orientation == GTK_ORIENTATION_HORIZONTAL)
        *y += height;
    else
        *x += width;
}

static void
secondary_activate_cb(GObject *source_object,
                      GAsyncResult *res,
                      gpointer user_data)
{
    KiranSnIcon *icon;

    icon = KIRAN_SN_ICON(user_data);

    kiran_sn_item_gen_call_secondary_activate_finish(icon->priv->proxy, res, NULL);
}

static void
krian_sn_icon_secondary_activate(KiranSnIcon *icon,
                                 gint x,
                                 gint y)
{
    kiran_sn_item_gen_call_secondary_activate(icon->priv->proxy,
                                              x, y, NULL,
                                              secondary_activate_cb, icon);
}

static gboolean
kiran_sn_icon_button_press_event(GtkWidget *widget,
                                 GdkEventButton *event)
{
    KiranSnIcon *icon;
    KiranSnIconPrivate *priv;
    gint x;
    gint y;

    if (event->button < 2 || event->button > 3)
        return GTK_WIDGET_CLASS(kiran_sn_icon_parent_class)->button_press_event(widget, event);

    icon = KIRAN_SN_ICON(widget);
    priv = KIRAN_SN_ICON_GET_PRIVATE(icon);

    kiran_sn_icon_get_action_coordinates(icon, &x, &y);

    if (event->button == 2)
    {
        gdk_seat_ungrab(gdk_device_get_seat(event->device));
        krian_sn_icon_secondary_activate(icon, x, y);
    }
    else if (event->button == 3)
    {
        if ((g_strcmp0(priv->menu, "/NO_DBUSMENU") != 0) && priv->gmenu != NULL)
        {
            gtk_menu_popup_at_widget(priv->gmenu, widget,
                                     GDK_GRAVITY_SOUTH_WEST,
                                     GDK_GRAVITY_NORTH_WEST,
                                     (GdkEvent *)event);
            /*Fix positioning if size changed since last shown*/
            gtk_menu_reposition(priv->gmenu);
        }
        else
        {
            gdk_seat_ungrab(gdk_device_get_seat(event->device));
            kiran_sn_icon_context_menu(icon, x, y);
        }
    }
    else
    {
        g_assert_not_reached();
    }

    return TRUE;
}

static void
activate_cb(GObject *source_object,
            GAsyncResult *res,
            gpointer user_data)
{
    KiranSnIcon *icon;

    icon = KIRAN_SN_ICON(user_data);

    kiran_sn_item_gen_call_activate_finish(icon->priv->proxy, res, NULL);
}

static void
kiran_sn_icon_clicked(GtkButton *button)
{
    KiranSnIcon *icon;
    gint x;
    gint y;

    icon = KIRAN_SN_ICON(button);

    kiran_sn_icon_get_action_coordinates(icon, &x, &y);

    kiran_sn_item_gen_call_activate(icon->priv->proxy,
                                    x, y, NULL,
                                    activate_cb, icon);
}

static gboolean
kiran_sn_icon_enter_notify_event(GtkWidget *widget,
                                 GdkEventCrossing *event)
{
    gtk_widget_grab_focus(widget);
    return GTK_WIDGET_CLASS(kiran_sn_icon_parent_class)->enter_notify_event(widget, event);
}

static void
kiran_sn_icon_class_init(KiranSnIconClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
    GtkButtonClass *button_class = GTK_BUTTON_CLASS(klass);

    gobject_class->constructed = kiran_sn_icon_constructed;
    gobject_class->get_property = kiran_sn_icon_get_property;
    gobject_class->set_property = kiran_sn_icon_set_property;
    gobject_class->dispose = kiran_sn_icon_dispose;
    gobject_class->finalize = kiran_sn_icon_finalize;

    widget_class->button_press_event = kiran_sn_icon_button_press_event;
    widget_class->enter_notify_event = kiran_sn_icon_enter_notify_event;
    button_class->clicked = kiran_sn_icon_clicked;

    install_properites(gobject_class);

    signals[SIGNAL_READY] =
        g_signal_new("ready",
                     G_TYPE_FROM_CLASS(gobject_class),
                     G_SIGNAL_RUN_LAST,
                     0, NULL, NULL, NULL,
                     G_TYPE_NONE, 0);

    g_type_class_add_private(gobject_class, sizeof(KiranSnIconPrivate));
}

/**
 * NOTE:
 * 直接使用gtk_widget_set_tooltip_markup
 * 将会触发 gtk_widget_queue_tooltip_query
 * 存在几率Popup已弹出但触发Tooltip显示，后续由于无法拿到鼠标事件导致该提示框不会消失
 *
 * 直接连接GtkWidget::query-tooltip进行处理
*/
static gboolean kiran_sn_icon_query_tooltip(GtkWidget *widget,
                                            gint x, gint y,
                                            gboolean keyboard_mode,
                                            GtkTooltip *tooltip, gpointer user_data)
{
    KiranSnIcon *icon = NULL;
    KiranSnTooltip *tip = NULL;
    gchar *markup = NULL;

    icon = KIRAN_SN_ICON(user_data);
    tip = icon->priv->tooltip;

    if (!icon->priv->tooltip)
        return FALSE;

    if ((tip->title != NULL && *tip->title != '\0') &&
        (tip->text != NULL && *tip->text != '\0'))
    {
        markup = g_strdup_printf("%s\n%s", tip->title, tip->text);
    }
    else if (tip->title != NULL && *tip->title != '\0')
    {
        markup = g_strdup(tip->title);
    }
    else if (tip->text != NULL && *tip->text != '\0')
    {
        markup = g_strdup(tip->text);
    }

    gtk_tooltip_set_markup(tooltip,markup);
    g_free(markup);
    return TRUE;
}

static void
kiran_sn_icon_init(KiranSnIcon *self)
{
    KiranSnIconPrivate *priv;

    priv = self->priv = KIRAN_SN_ICON_GET_PRIVATE(self);
    priv->icon_size = 16;
    priv->effective_icon_size = 16;
    priv->update_id = 0;
    priv->id = NULL;
    priv->icon_name = NULL;
    priv->category = KIRAN_NOTIFY_ICON_CATEGORY_APPLICATION_STATUS;
    priv->app_category = NULL;
    priv->icon = NULL;

    g_signal_connect(GTK_WIDGET(self), "query-tooltip", G_CALLBACK(kiran_sn_icon_query_tooltip), self);
    gtk_widget_set_has_tooltip(GTK_WIDGET(self), TRUE);

    priv->image = gtk_image_new();
    gtk_container_add(GTK_CONTAINER(self), priv->image);
    gtk_widget_show(priv->image);
}

const gchar *
kiran_sn_icon_get_bus_name(KiranSnIcon *icon)
{
    KiranSnIconPrivate *priv = icon->priv;

    return priv->bus_name;
}

const gchar *
kiran_sn_icon_get_object_path(KiranSnIcon *icon)
{
    KiranSnIconPrivate *priv = icon->priv;

    return priv->object_path;
}

void kiran_sn_icon_set_icon_size(KiranSnIcon *icon,
                                 gint icon_size)
{
    KiranSnIconPrivate *priv = icon->priv;

    priv->icon_size = icon_size;

    queue_update(icon);
}

GtkWidget *
kiran_sn_icon_new(const gchar *bus_name,
                  const gchar *object_path,
                  gint icon_size)
{
    return g_object_new(KIRAN_TYPE_SN_ICON,
                        "bus-name", bus_name,
                        "object-path", object_path,
                        "icon-size", icon_size,
                        NULL);
}
