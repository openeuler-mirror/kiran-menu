#include "kiran-app-wrapper.h"

/**
 * 检查给定的app是否位于收藏夹中 
 *
 */
gboolean kiran_app_is_favorite(KiranApp *app)
{
    KiranApp *result;
    KiranMenuBased *backend = kiran_menu_based_skeleton_get();
    gboolean res = FALSE;

    result = kiran_menu_based_lookup_favorite_app(backend, kiran_app_get_desktop_id(app));
    if (result) {
        res = TRUE;
        g_object_unref(result);
    }

    return res;
}


/**
 * 将给定app添加到收藏夹中
 */
gboolean kiran_app_add_to_favorite(KiranApp *app)
{
    KiranApp *result;
    KiranMenuBased *backend = kiran_menu_based_skeleton_get();
    gboolean res = FALSE;

    if (kiran_app_is_favorite(app))
        return TRUE;

    return kiran_menu_based_add_favorite_app(backend, kiran_app_get_desktop_id(app));
}

/**
 * 将给定app从收藏夹中删除
 * 
 * @return 如果app不再收藏夹中或删除成功，返回TRUE, 否则返回FALSE
 */
gboolean kiran_app_remove_from_favorite(KiranApp *app)
{
    KiranApp *result;
    KiranMenuBased *backend = kiran_menu_based_skeleton_get();
    gboolean res = FALSE;

    if (!kiran_app_is_favorite(app))
        return TRUE;

    return kiran_menu_based_del_favorite_app(backend, kiran_app_get_desktop_id(app));
}

/**
 * 将指定的app添加到桌面
 */
gboolean kiran_app_add_to_desktop(KiranApp *app)
{
    g_autofree gchar* command = NULL;
    GError *error = NULL;
    gint status;
    gboolean result = FALSE;

    command = g_strdup_printf("xdg-desktop-icon install %s --novendor", kiran_app_get_file_name(app));
    g_assert(command != NULL);

    if (!g_spawn_command_line_sync(command, NULL, NULL, &status, &error)) {
        g_warning("Failed to call command: %s\n", error->message);
        g_error_free(error);
    } else {
        if (status) {
            g_warning("command '%s' exited with code %d\n", command, status);
        } else
            result = TRUE;
    }

    return result;
}