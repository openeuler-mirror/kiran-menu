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

#include "kiran-tray.h"
#include <fcntl.h>
#include <gdk/gdkx.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <libxml/parser.h>
#include <libxml/xmlmemory.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "kiran-notify-icon.h"
#include "kiran-sn-tray-manager.h"
#include "kiran-x11-tray-manager.h"
#include "kiran-x11-tray-socket.h"

#define ROOT_NODE_NAME "apps"
#define APP_NODE_NAME "app"
#define APP_ID_NODE_NAME "id"
#define APP_ICON_NODE_NAME "icon"         /*应用图标 */
#define APP_TITLE_NODE_NAME "title"       /*应用名称*/
#define APP_CATEGORY_NODE_NAME "category" /*应用分类 */

struct _KiranTrayPrivate
{
    GSList *icons;
    GSList *managers;
    GSettings *settings;
};

static void kiran_tray_dispose(GObject *object);
static void kiran_tray_finalize(GObject *object);
static GObject *kiran_tray_constructor(GType type,
                                       guint n_construct_properties,
                                       GObjectConstructParam *construct_properties);
static void kiran_tray_realize(GtkWidget *widget);
static void kiran_tray_unrealize(GtkWidget *widget);

static void kiran_tray_add_manager(KiranTray *tray, KiranTrayManager *manager);
static void kiran_tray_icons_refresh(KiranTray *tray);
static void kiran_tray_style_updated(GtkWidget *widget);

G_DEFINE_TYPE_WITH_PRIVATE(KiranTray, kiran_tray, GTK_TYPE_BOX)

static void
kiran_tray_set_icon_size(KiranTray *tray)
{
    KiranTrayPrivate *priv;
    GSList *iterator = NULL;
    gint icon_size = 0;

    priv = tray->priv;
    icon_size = g_settings_get_int(priv->settings, KEY_TRAY_ICON_SIZE);

    for (iterator = priv->managers; iterator; iterator = iterator->next)
    {
        kiran_tray_manager_set_icon_size(iterator->data, icon_size);
    }
}

static void
kiran_tray_set_icon_padding(KiranTray *tray)
{
    KiranTrayPrivate *priv;
    gint icon_padding = 2;

    priv = tray->priv;
    icon_padding = g_settings_get_int(priv->settings, KEY_TRAY_ICON_PADDING);

    gtk_box_set_spacing(GTK_BOX(tray), icon_padding);
}

static void
gsettings_changed_panel_icon_size(GSettings *settings,
                                  gchar *key,
                                  KiranTray *tray)
{
    kiran_tray_set_icon_size(tray);
}

static void
gsettings_changed_panel_icon_padding(GSettings *settings,
                                     gchar *key,
                                     KiranTray *tray)
{
    kiran_tray_set_icon_padding(tray);
}

static void
kiran_tray_class_init(KiranTrayClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

    gobject_class->dispose = kiran_tray_dispose;
    gobject_class->finalize = kiran_tray_finalize;
    gobject_class->constructor = kiran_tray_constructor;
    widget_class->realize = kiran_tray_realize;
    widget_class->unrealize = kiran_tray_unrealize;
    widget_class->style_updated = kiran_tray_style_updated;
}

static void
kiran_tray_init(KiranTray *self)
{
    GdkDisplay *display;
    GtkCssProvider *provider;
    GdkScreen *screen;
    KiranTrayPrivate *priv;

    priv = self->priv = kiran_tray_get_instance_private(self);
    priv->settings = g_settings_new(KIRAN_TRAY_SCHEMA);
    g_signal_connect(priv->settings, "changed::" KEY_TRAY_ICON_SIZE, G_CALLBACK(gsettings_changed_panel_icon_size), self);
    g_signal_connect(priv->settings, "changed::" KEY_TRAY_ICON_PADDING, G_CALLBACK(gsettings_changed_panel_icon_padding), self);

    provider = gtk_css_provider_new();
    display = gdk_display_get_default();
    screen = gdk_display_get_default_screen(display);
    gtk_style_context_add_provider_for_screen(screen,
                                              GTK_STYLE_PROVIDER(provider),
                                              GTK_STYLE_PROVIDER_PRIORITY_USER);
    gtk_css_provider_load_from_resource(provider, "/kiran-applet/css/applet.css");

    gtk_widget_set_halign(GTK_WIDGET(self), GTK_ALIGN_START);
    gtk_widget_set_hexpand(GTK_WIDGET(self), FALSE);
    g_object_unref(provider);
}

static void
kiran_tray_icons_refresh(KiranTray *tray)
{
    GList *children;
    GList *child;
    gint col = 0;

    children = gtk_container_get_children(GTK_CONTAINER(tray));
    children = g_list_sort(children, (GCompareFunc)kiran_notify_icon_compare);

    for (child = children; child; child = child->next)
    {
        gtk_container_child_set(GTK_CONTAINER(tray),
                                child->data,
                                "position", col,
                                NULL);
        col++;
    }
    g_list_free(children);
}

static void
kiran_tray_dispose(GObject *object)
{
    KiranTray *tray = KIRAN_TRAY(object);
    KiranTrayPrivate *priv = tray->priv;

    /* 在 dispose 阶段而不是 finalize 阶段断开信号连接：
     * - GSettings 的 changed 信号可能通过 idle 回调延迟触发
     * - 如果在 finalize 才断开，idle 回调可能已在队列中
     * - dispose 是对象销毁的第一个阶段，此时断开可以防止回调访问无效对象
     */

    if (priv->settings != NULL)
    {
        g_signal_handlers_disconnect_by_func(priv->settings,
                                             G_CALLBACK(gsettings_changed_panel_icon_size),
                                             tray);

        g_signal_handlers_disconnect_by_func(priv->settings,
                                             G_CALLBACK(gsettings_changed_panel_icon_padding),
                                             tray);
        g_object_unref(priv->settings);
        priv->settings = NULL;
    }

    G_OBJECT_CLASS(kiran_tray_parent_class)->dispose(object);
}

static void
kiran_tray_finalize(GObject *object)
{
    G_OBJECT_CLASS(kiran_tray_parent_class)->finalize(object);
}

static GObject *
kiran_tray_constructor(GType type,
                       guint n_construct_properties,
                       GObjectConstructParam *construct_properties)
{
    GObject *obj;
    KiranTray *tray;

    obj = G_OBJECT_CLASS(kiran_tray_parent_class)->constructor(type, n_construct_properties, construct_properties);
    tray = KIRAN_TRAY(obj);
    gtk_box_set_homogeneous(GTK_BOX(tray), FALSE);
    gtk_widget_set_name(GTK_WIDGET(tray), "trayBox");
    kiran_tray_set_icon_padding(tray);

    return obj;
}

static void
kiran_tray_realize(GtkWidget *widget)
{
    KiranTray *tray = KIRAN_TRAY(widget);
    KiranTrayManager *manager;

    GTK_WIDGET_CLASS(kiran_tray_parent_class)->realize(widget);

    manager = kiran_x11_tray_manager_new(gtk_widget_get_screen(widget));

    kiran_tray_add_manager(tray, manager);

    manager = kiran_sn_tray_manager_new();
    kiran_tray_add_manager(tray, manager);

    kiran_tray_set_icon_size(tray);
}

static void
kiran_tray_unrealize(GtkWidget *widget)
{
    KiranTrayPrivate *priv = KIRAN_TRAY(widget)->priv;

    if (priv->managers)
    {
        g_slist_free_full(priv->managers, g_object_unref);
        priv->managers = NULL;
    }

    g_clear_pointer(&priv->icons, g_slist_free);

    GTK_WIDGET_CLASS(kiran_tray_parent_class)->unrealize(widget);
}

static void
kiran_tray_style_updated(GtkWidget *widget)
{
    KiranTray *tray;
    KiranTrayPrivate *priv;
    GSList *iterator = NULL;
    GtkStyleContext *context;

    if (GTK_WIDGET_CLASS (kiran_tray_parent_class)->style_updated)
        GTK_WIDGET_CLASS (kiran_tray_parent_class)->style_updated (widget);

    tray = KIRAN_TRAY (widget);
    priv = tray->priv;
    context = gtk_widget_get_style_context (widget);

    for (iterator = priv->managers; iterator; iterator = iterator->next)
    {
        kiran_tray_manager_style_updated (iterator->data, context);
    }
}

static gboolean
kiran_tray_icon_is_hidden(GSettings *settings, const char *id)
{
    gchar **hide_icon_ids;
    gboolean hidden = FALSE;
    gint i;

    if (!id)
        return FALSE;

    hide_icon_ids = g_settings_get_strv(settings, KEY_HIDE_ICON_IDS);
    if (hide_icon_ids)
    {
        for (i = 0; hide_icon_ids[i]; i++)
        {
            if (g_strcmp0(id, hide_icon_ids[i]) == 0)
            {
                hidden = TRUE;
                break;
            }
        }
        g_strfreev(hide_icon_ids);
    }

    return hidden;
}

static xmlNodePtr
create_app_node(KiranNotifyIcon *icon)
{
    xmlNodePtr node;
    KiranNotifyIconCategory icon_cate;
    const char *id;
    const char *icon_name;
    const char *name;
    const char *app_category;

    id = kiran_notify_icon_get_id(icon);
    icon_name = kiran_notify_icon_get_icon(icon);
    name = kiran_notify_icon_get_name(icon);
    app_category = kiran_notify_icon_get_app_category(icon);

    node = xmlNewNode(NULL, (const xmlChar *)APP_NODE_NAME);
    xmlNewTextChild(node, NULL, (const xmlChar *)APP_ID_NODE_NAME, (const xmlChar *)id);
    xmlNewTextChild(node, NULL, (const xmlChar *)APP_ICON_NODE_NAME, (const xmlChar *)icon_name);
    xmlNewTextChild(node, NULL, (const xmlChar *)APP_TITLE_NODE_NAME, (const xmlChar *)name);
    xmlNewTextChild(node, NULL, (const xmlChar *)APP_CATEGORY_NODE_NAME, (const xmlChar *)app_category);

    return node;
}

static void
record_tray_notify_icon(KiranNotifyIcon *icon)
{
    gchar *docname = NULL;
    xmlDocPtr doc;
    xmlNodePtr root_node;
    xmlNodePtr node;
    const char *id;

    id = kiran_notify_icon_get_id(icon);

    docname = g_strdup_printf("%s/.config/kiran-tray/apps.xml", g_get_home_dir());
    if (docname == NULL)
        return;

    if (!g_file_test(docname, G_FILE_TEST_EXISTS))
    {
        //文件不存在
        gchar *dir = NULL;

        dir = g_strdup_printf("%s/.config", g_get_home_dir());
        if (dir == NULL)
        {
            g_free(docname);
            return;
        }

        if (!g_file_test(dir, G_FILE_TEST_EXISTS))
        {
            g_mkdir(dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        }
        free(dir);

        dir = g_strdup_printf("%s/.config/kiran-tray", g_get_home_dir());
        if (dir == NULL)
        {
            g_free(docname);
            return;
        }

        if (!g_file_test(dir, G_FILE_TEST_EXISTS))
        {
            g_mkdir(dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        }
        free(dir);
    }

    doc = NULL;
    if (g_file_test(docname, G_FILE_TEST_EXISTS))
        doc = xmlReadFile(docname, NULL, XML_PARSE_NOBLANKS);

    if (doc == NULL)
    {
        doc = xmlNewDoc((const xmlChar *)"1.0");
        root_node = xmlNewNode(NULL, (const xmlChar *)ROOT_NODE_NAME);
        node = create_app_node(icon);
        xmlAddChild(root_node, node);

        xmlDocSetRootElement(doc, root_node);
        xmlSaveFormatFileEnc(docname, doc, "UTF-8", 1);
    }
    else
    {
        root_node = xmlDocGetRootElement(doc);
        if (root_node == NULL)
        {
            if (g_remove(docname) == 0)
            {
                record_tray_notify_icon(icon);
            }
        }
        else
        {
            if (xmlStrcmp(root_node->name, (const xmlChar *)ROOT_NODE_NAME))
            {
                record_tray_notify_icon(icon);
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
                                xmlChar *content = xmlNodeGetContent(id_node);
                                if (content != NULL)
                                {
                                    is_exist = !xmlStrcmp(content, (const xmlChar *)id);
                                    xmlFree(content);
                                    if (is_exist)
                                    {
                                        break;
                                    }
                                }
                            }
                            id_node = id_node->next;
                        }
                    }

                    node = node->next;
                }

                if (!is_exist)
                {
                    node = create_app_node(icon);
                    xmlAddChild(root_node, node);
                    xmlSaveFormatFileEnc(docname, doc, "UTF-8", 1);
                }
            }
        }
    }

    //关闭文档指针，并清除文档中所有节点动态申请的内存
    xmlFreeDoc(doc);
    g_free(docname);
}

static void
kiran_tray_notify_icon_map_callback(GtkWidget *widget,
                                    gpointer user_data)
{
    KiranNotifyIcon *icon = KIRAN_NOTIFY_ICON(widget);

    /* 当窗口保存图标时才进行记录 */
    if (kiran_notify_icon_get_icon(icon))
    {
        record_tray_notify_icon(icon);
    }
}

static void
kiran_tray_notify_icon_added(KiranTrayManager *manager,
                             KiranNotifyIcon *icon,
                             KiranTray *tray)
{
    KiranTrayPrivate *priv;
    const char *id;

    g_return_if_fail(KIRAN_IS_TRAY_MANAGER(manager));
    g_return_if_fail(KIRAN_IS_NOTIFY_ICON(icon));
    g_return_if_fail(KIRAN_IS_TRAY(tray));

    priv = tray->priv;

    id = kiran_notify_icon_get_id(icon);

    /* 系统已知的通知图标如声音，网络，电池等不需要记录 */
    if (kiran_notify_icon_get_category(icon) != KIRAN_NOTIFY_ICON_CATEGORY_HARDWARE)
    {
        g_signal_connect(icon,
                         "map",
                         G_CALLBACK(kiran_tray_notify_icon_map_callback),
                         NULL);
    }

    gtk_widget_set_name (GTK_WIDGET (icon), "iconWinButton");

    if (!kiran_tray_icon_is_hidden(priv->settings, id))
    {
        gtk_box_pack_start(GTK_BOX(tray), GTK_WIDGET(icon), FALSE, TRUE, 0);
        kiran_tray_icons_refresh(tray);
    }

    priv->icons = g_slist_prepend(priv->icons, icon);
}

static void
kiran_tray_notify_icon_removed(KiranTrayManager *manager,
                               KiranNotifyIcon *icon,
                               KiranTray *tray)
{
    KiranTrayPrivate *priv;

    g_return_if_fail(KIRAN_IS_TRAY_MANAGER(manager));
    g_return_if_fail(KIRAN_IS_NOTIFY_ICON(icon));
    g_return_if_fail(KIRAN_IS_TRAY(tray));

    priv = tray->priv;

    /* 隐藏区域已删除, 图标只可能挂在面板容器上; 按实际父容器删除, 避免删到无子关系容器的告警 */
    if (gtk_widget_get_parent(GTK_WIDGET(icon)) == GTK_WIDGET(tray))
        gtk_container_remove(GTK_CONTAINER(tray), GTK_WIDGET(icon));

    priv->icons = g_slist_remove(priv->icons, icon);
}

static void
kiran_tray_add_manager(KiranTray *tray, KiranTrayManager *manager)
{
    KiranTrayPrivate *priv = tray->priv;

    priv->managers = g_slist_prepend(priv->managers, manager);

    g_signal_connect_object(manager, "icon-added",
                            G_CALLBACK(kiran_tray_notify_icon_added), tray, 0);
    g_signal_connect_object(manager, "icon-removed",
                            G_CALLBACK(kiran_tray_notify_icon_removed), tray, 0);
}

GtkWidget *
kiran_tray_new(void)
{
    return g_object_new(KIRAN_TYPE_TRAY, NULL);
}
