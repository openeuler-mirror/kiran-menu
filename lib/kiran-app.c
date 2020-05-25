/*
 * @Author       : tangjie02
 * @Date         : 2020-04-08 14:10:38
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-05-25 09:57:19
 * @Description  :
 * @FilePath     : /kiran-menu-2.0/lib/kiran-app.c
 */

#include "lib/kiran-app.h"

#include <gio/gdesktopappinfo.h>

#include "lib/kiran-menu-app.h"
#include "lib/helper.h"

typedef struct _KiranAppPrivate KiranAppPrivate;

struct _KiranAppPrivate
{
    GObject parent;

    gchar *desktop_id;

    gchar *file_name;

    gchar *name;
    gchar *locale_name;

    gchar *comment;
    gchar *locale_comment;

    gchar *exec;

    gchar *icon_name;

    gchar *path;

    KiranAppKind kind;

    GDesktopAppInfo *desktop_app;
};

G_DEFINE_TYPE_WITH_PRIVATE(KiranApp, kiran_app, G_TYPE_OBJECT);

enum
{
    PROP_NONE,
    PROP_DESKTOP_ID,
};

enum
{
    SIGNAL_LAUNCHED = 0,
    SIGANL_LAUNCH_FAILED,
    SIGNAL_LAST
};

static guint signals[SIGNAL_LAST];

const gchar *kiran_app_get_name(KiranApp *self)
{
    g_return_val_if_fail(self != NULL, NULL);

    KiranAppPrivate *priv = kiran_app_get_instance_private(self);

    return priv->name;
}

const gchar *kiran_app_get_locale_name(KiranApp *self)
{
    g_return_val_if_fail(self != NULL, NULL);

    KiranAppPrivate *priv = kiran_app_get_instance_private(self);

    return priv->locale_name;
}

const gchar *kiran_app_get_comment(KiranApp *self)
{
    g_return_val_if_fail(self != NULL, NULL);

    KiranAppPrivate *priv = kiran_app_get_instance_private(self);

    return priv->comment;
}

const gchar *kiran_app_get_locale_comment(KiranApp *self)
{
    g_return_val_if_fail(self != NULL, NULL);

    KiranAppPrivate *priv = kiran_app_get_instance_private(self);

    return priv->locale_comment;
}

const gchar *kiran_app_get_desktop_id(KiranApp *self)
{
    g_return_val_if_fail(self != NULL, NULL);

    KiranAppPrivate *priv = kiran_app_get_instance_private(self);

    return priv->desktop_id;
}

const gchar *kiran_app_get_exec(KiranApp *self)
{
    g_return_val_if_fail(self != NULL, NULL);

    KiranAppPrivate *priv = kiran_app_get_instance_private(self);

    return priv->exec;
}

const gchar *kiran_app_get_categories(KiranApp *self)
{
    g_return_val_if_fail(self != NULL, NULL);

    KiranAppPrivate *priv = kiran_app_get_instance_private(self);
    g_return_val_if_fail(priv->desktop_app != NULL, NULL);

    return g_desktop_app_info_get_categories(priv->desktop_app);
}

const gchar *kiran_app_get_file_name(KiranApp *self)
{
    g_return_val_if_fail(self != NULL, NULL);

    KiranAppPrivate *priv = kiran_app_get_instance_private(self);
    g_return_val_if_fail(priv->desktop_app != NULL, NULL);

    return g_desktop_app_info_get_filename(priv->desktop_app);
}

guint64 kiran_app_get_create_time(KiranApp *self)
{
    g_return_val_if_fail(self != NULL, 0);

    KiranAppPrivate *priv = kiran_app_get_instance_private(self);

    const char *full_path = g_desktop_app_info_get_filename(priv->desktop_app);
    g_return_val_if_fail(full_path != NULL, 0);

    g_autoptr(GFile) file = g_file_new_for_path(full_path);
    g_autoptr(GFileInfo) file_info = g_file_query_info(
        file, G_FILE_ATTRIBUTE_TIME_CREATED, G_FILE_QUERY_INFO_NONE, NULL, NULL);

    if (file_info)
    {
        return g_file_info_get_attribute_uint64(file_info,
                                                G_FILE_ATTRIBUTE_TIME_CREATED);
    }
    return 0;
}

static void kiran_app_init(KiranApp *self)
{
}

static void kiran_app_get_property(GObject *gobject, guint prop_id,
                                   GValue *value, GParamSpec *pspec)
{
    KiranApp *app = KIRAN_APP(gobject);

    KiranAppPrivate *priv = kiran_app_get_instance_private(app);

    switch (prop_id)
    {
    case PROP_DESKTOP_ID:
        g_value_set_string(value, priv->desktop_id);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, pspec);
        break;
    }
}

static KiranAppKind _kiran_app_get_kind(KiranApp *self)
{
    KiranAppPrivate *priv = kiran_app_get_instance_private(self);

    RETURN_VAL_IF_FALSE(priv != NULL, KIRAN_APP_KIND_UNKNOWN);
    RETURN_VAL_IF_FALSE(priv->exec != NULL, KIRAN_APP_KIND_UNKNOWN);

    g_auto(GStrv) exec_split = g_strsplit(priv->exec, " ", -1);

    if (!exec_split || !(exec_split[0]))
    {
        return KIRAN_APP_KIND_UNKNOWN;
    }

    g_autofree gchar *exec_name = NULL;

    exec_name = g_path_get_basename(exec_split[0]);
    if (g_strcmp0(exec_name, "flatpak") == 0)
    {
        return KIRAN_APP_KIND_FLATPAK;
    }

    g_autofree gchar *x_flatpak = g_desktop_app_info_get_string(priv->desktop_app, "X-Flatpak");
    if (x_flatpak)
    {
        return KIRAN_APP_KIND_FLATPAK;
    }

    return KIRAN_APP_KIND_DESKTOP;
}

static void kiran_app_set_app_info(KiranApp *app, const char *desktop_id)
{
    KiranAppPrivate *priv = kiran_app_get_instance_private(app);

    priv->desktop_id = g_strdup(desktop_id);
    priv->desktop_app = g_desktop_app_info_new(desktop_id);

    priv->file_name = g_strdup(g_desktop_app_info_get_filename(priv->desktop_app));

#define GET_STRING(key) g_desktop_app_info_get_string(priv->desktop_app, key)
#define GET_LOCALE_STRING(key) \
    g_desktop_app_info_get_locale_string(priv->desktop_app, key)

    priv->name = GET_STRING("Name");
    priv->locale_name = GET_LOCALE_STRING("Name");

    priv->comment = GET_STRING("Comment");
    priv->locale_comment = GET_LOCALE_STRING("Comment");

    priv->exec = GET_STRING("Exec");

    priv->icon_name = GET_STRING("Icon");

    priv->path = GET_STRING("Path");

    if (priv->path && priv->path[0] == '\0')
    {
        g_free(priv->path);
        priv->path = NULL;
    }

    priv->kind = _kiran_app_get_kind(app);

#undef GET_STRING
#undef GET_LOCALE_STRING
}

static void kiran_app_set_property(GObject *gobject, guint prop_id,
                                   const GValue *value, GParamSpec *pspec)
{
    KiranApp *app = KIRAN_APP(gobject);

    switch (prop_id)
    {
    case PROP_DESKTOP_ID:
        kiran_app_set_app_info(app, g_value_get_string(value));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, pspec);
        break;
    }
}

static void kiran_app_dispose(GObject *object)
{
    KiranApp *app = KIRAN_APP(object);

    KiranAppPrivate *priv = kiran_app_get_instance_private(app);

    g_clear_pointer(&priv->desktop_id, g_free);
    g_clear_object(&priv->desktop_app);

    g_clear_pointer(&priv->file_name, g_free);

    g_clear_pointer(&priv->name, g_free);
    g_clear_pointer(&priv->locale_name, g_free);

    g_clear_pointer(&priv->comment, g_free);
    g_clear_pointer(&priv->locale_comment, g_free);

    g_clear_pointer(&priv->exec, g_free);

    g_clear_pointer(&priv->icon_name, g_free);

    g_clear_pointer(&priv->path, g_free);

    G_OBJECT_CLASS(kiran_app_parent_class)->dispose(object);
}

static void kiran_app_class_init(KiranAppClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    object_class->get_property = kiran_app_get_property;
    object_class->set_property = kiran_app_set_property;
    object_class->dispose = kiran_app_dispose;

    g_object_class_install_property(
        object_class, PROP_DESKTOP_ID,
        g_param_spec_string(
            "desktop-id", "Application id", "The desktop file id", NULL,
            G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

    signals[SIGNAL_LAUNCHED] = g_signal_new("launched",
                                            KIRAN_TYPE_APP,
                                            G_SIGNAL_RUN_LAST,
                                            0,
                                            NULL,
                                            NULL,
                                            NULL,
                                            G_TYPE_NONE,
                                            0);

    signals[SIGANL_LAUNCH_FAILED] = g_signal_new("launch-failed",
                                                 KIRAN_TYPE_APP,
                                                 G_SIGNAL_RUN_LAST,
                                                 0,
                                                 NULL,
                                                 NULL,
                                                 NULL,
                                                 G_TYPE_NONE,
                                                 0);
}

GIcon *kiran_app_get_icon(KiranApp *self)
{
    KiranAppPrivate *priv = kiran_app_get_instance_private(self);

    return g_app_info_get_icon(G_APP_INFO(priv->desktop_app));
}

KiranAppKind kiran_app_get_kind(KiranApp *self)
{
    KiranAppPrivate *priv = kiran_app_get_instance_private(self);

    return priv->kind;
}

typedef struct
{
    GSpawnChildSetupFunc user_setup;
    gpointer user_setup_data;

    char *pid_envvar;
} ChildSetupData;

static void expand_macro(KiranApp *self,
                         char macro,
                         GString *exec)
{
    KiranAppPrivate *priv = kiran_app_get_instance_private(self);

    char *expanded = NULL;

    g_return_if_fail(exec != NULL);

    switch (macro)
    {
    case 'i':
        if (priv->icon_name)
        {
            g_string_append(exec, "--icon ");
            expanded = g_shell_quote(priv->icon_name);
            g_string_append(exec, expanded);
            g_free(expanded);
        }
        break;

    case 'c':
        if (priv->locale_name)
        {
            expanded = g_shell_quote(priv->locale_name);
            g_string_append(exec, expanded);
            g_free(expanded);
        }
        break;

    case 'k':
        if (priv->file_name)
        {
            expanded = g_shell_quote(priv->file_name);
            g_string_append(exec, expanded);
            g_free(expanded);
        }
        break;

    case '%':
        g_string_append_c(exec, '%');
        break;
    }
}

static gboolean expand_application_parameters(KiranApp *self,
                                              int *argc,
                                              char ***argv,
                                              GError **error)
{
    KiranAppPrivate *priv = kiran_app_get_instance_private(self);

    const char *p = priv->exec;
    GString *expanded_exec;
    gboolean res;

    if (priv->exec == NULL)
    {
        g_set_error_literal(error,
                            G_IO_ERROR,
                            G_IO_ERROR_FAILED,
                            "Desktop file didn’t specify Exec field");
        return FALSE;
    }

    expanded_exec = g_string_new(NULL);

    while (*p)
    {
        if (p[0] == '%' && p[1] != '\0')
        {
            expand_macro(self, p[1], expanded_exec);
            p++;
        }
        else
            g_string_append_c(expanded_exec, *p);

        p++;
    }
    res = g_shell_parse_argv(expanded_exec->str, argc, argv, error);
    g_string_free(expanded_exec, TRUE);
    return res;
}

static void child_setup(gpointer user_data)
{
    gchar *pid_envvar = (gchar *)user_data;

    if (pid_envvar)
    {
        pid_t pid = getpid();
        char buf[20];
        int i;

        /* Write the pid into the space already reserved for it in the
       * environment array. We can't use sprintf because it might
       * malloc, so we do it by hand. It's simplest to write the pid
       * out backwards first, then copy it over.
       */
        for (i = 0; pid; i++, pid /= 10)
            buf[i] = (pid % 10) + '0';
        for (i--; i >= 0; i--)
            *(pid_envvar++) = buf[i];
        *pid_envvar = '\0';
    }
}

static gboolean kiran_app_launch_flatpak(KiranApp *self,
                                         GError **error)
{
    g_return_val_if_fail(self != NULL, FALSE);

    KiranAppPrivate *priv = kiran_app_get_instance_private(self);

    gboolean completed = FALSE;

    char **argv, **envp;
    int argc;
    gchar *pid_envvar;

    argv = NULL;
    envp = g_get_environ();

    GPid pid;
    GList *iter;

    if (!expand_application_parameters(self, &argc, &argv, error))
        goto out;

    if (priv->file_name)
    {
        envp = g_environ_setenv(envp,
                                "GIO_LAUNCHED_DESKTOP_FILE",
                                priv->file_name,
                                TRUE);
        envp = g_environ_setenv(envp,
                                "GIO_LAUNCHED_DESKTOP_FILE_PID",
                                "XXXXXXXXXXXXXXXXXXXX", /* filled in child_setup */
                                TRUE);
        pid_envvar = (char *)g_environ_getenv(envp, "GIO_LAUNCHED_DESKTOP_FILE_PID");
    }
    else
    {
        pid_envvar = NULL;
    }

    if (!g_spawn_async(priv->path,
                       argv,
                       envp,
                       G_SPAWN_SEARCH_PATH,
                       child_setup,
                       pid_envvar,
                       &pid,
                       error))
    {
        goto out;
    }

    g_strfreev(argv);
    argv = NULL;

    completed = TRUE;

out:
    g_strfreev(argv);
    g_strfreev(envp);

    return completed;
}

gboolean kiran_app_launch(KiranApp *self)
{
    gboolean res = FALSE;
    g_autoptr(GError) error = NULL;
    KiranAppPrivate *priv = kiran_app_get_instance_private(self);

    if (priv->kind == KIRAN_APP_KIND_FLATPAK)
    {
        res = kiran_app_launch_flatpak(self, &error);
    }
    else
    {
        res = g_app_info_launch(G_APP_INFO(priv->desktop_app), NULL, NULL, &error);
    }

    if (res)
    {
        g_signal_emit(self, signals[SIGNAL_LAUNCHED], 0);
    }
    else
    {
        g_signal_emit(self, signals[SIGANL_LAUNCH_FAILED], 0);
        g_warning("Failed to launch: %s", error->message);
    }
    return res;
}