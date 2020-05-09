/*
 * @Author       : tangjie02
 * @Date         : 2020-04-08 14:01:52
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-05-09 10:56:20
 * @Description  : 用于菜单搜索功能
 * @FilePath     : /kiran-menu-2.0/lib/kiran-menu-search.c
 */
#include "lib/kiran-menu-search.h"

#include "lib/helper.h"
#include "lib/kiran-app.h"

struct _KiranMenuSearch
{
    GObject parent;
};

G_DEFINE_TYPE(KiranMenuSearch, kiran_menu_search, G_TYPE_OBJECT)

static gboolean strstr_with_case(const gchar *a, const gchar *b, gboolean ignore_case)
{
    RETURN_VAL_IF_FALSE(a != NULL, FALSE);
    RETURN_VAL_IF_FALSE(b != NULL, FALSE);

    if (g_strrstr(a, b) != NULL)
    {
        return TRUE;
    }

    if (ignore_case && g_str_is_ascii(a) && g_str_is_ascii(b))
    {
        g_autofree gchar *a_lower = str_tolower(a);
        g_autofree gchar *b_lower = str_tolower(b);
        return (g_strrstr(a_lower, b_lower) != NULL);
    }
    return FALSE;
}

GList *kiran_menu_search_by_keyword(KiranMenuSearch *self,
                                    const char *keyword,
                                    gboolean ignore_case,
                                    GList *apps)
{
    GList *match_apps = NULL;

    for (GList *l = apps; l != NULL; l = l->next)
    {
        KiranApp *app = l->data;
        const char *comment = kiran_app_get_comment(app);
        const char *locale_comment = kiran_app_get_locale_comment(app);
        const char *name = kiran_app_get_name(app);
        const char *locale_name = kiran_app_get_locale_name(app);

#define STRSTR_KEYWORD(a) (strstr_with_case(a, keyword, ignore_case))

        if (STRSTR_KEYWORD(name) || STRSTR_KEYWORD(locale_name) ||
            STRSTR_KEYWORD(comment) || STRSTR_KEYWORD(locale_comment))
        {
            match_apps = g_list_append(match_apps, g_object_ref(app));
        }

#undef STRSTR_KEYWORD
    }
    return match_apps;
}

static void kiran_menu_search_init(KiranMenuSearch *self) {}

static void kiran_menu_search_dispose(GObject *object)
{
    G_OBJECT_CLASS(kiran_menu_search_parent_class)->dispose(object);
}

static void kiran_menu_search_class_init(KiranMenuSearchClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    object_class->dispose = kiran_menu_search_dispose;
}

KiranMenuSearch *kiran_menu_search_get_new()
{
    return g_object_new(KIRAN_TYPE_MENU_SEARCH, NULL);
}
