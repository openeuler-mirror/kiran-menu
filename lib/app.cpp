/*
 * @Author       : tangjie02
 * @Date         : 2020-04-08 14:10:38
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-29 20:20:02
 * @Description  :
 * @FilePath     : /kiran-menu-2.0/lib/app.cpp
 */

#include "lib/app.h"

#include <vector>

#include "lib/helper.h"

namespace Kiran
{
App::App(const std::string &desktop_id)
{
    this->desktop_id_ = desktop_id;

    this->desktop_app_ = Gio::DesktopAppInfo::create(desktop_id);

    this->file_name_ = this->desktop_app_->get_filename();

#define GET_STRING(key) this->desktop_app_->get_string(key).raw()
#define GET_LOCALE_STRING(key) this->desktop_app_->get_locale_string(key).raw()

    this->name_ = GET_STRING("Name");
    this->locale_name_ = GET_LOCALE_STRING("Name");

    this->comment_ = GET_STRING("Comment");
    this->locale_comment_ = GET_LOCALE_STRING("Comment");

    this->exec_ = GET_STRING("Exec");

    this->icon_name_ = GET_STRING("Icon");

    this->path_ = GET_STRING("Path");

    this->x_kiran_no_display_ = this->desktop_app_->get_boolean("X-KIRAN-NoDisplay");

    init_app_kind();

#undef GET_STRING
#undef GET_LOCALE_STRING
}

App::~App()
{
}

std::string App::get_categories()
{
    return this->desktop_app_->get_categories();
}

const Glib::RefPtr<Gio::Icon> App::get_icon()
{
    return this->desktop_app_->get_icon();
}

bool App::launch()
{
    bool res = false;
    std::string error;

    // if (this->kind_ == AppKind::FLATPAK)
    // {
    //     GError *err;
    //     res = launch_flatpak(&err);
    //     if (!res)
    //     {
    //         error = err->message;
    //     }
    // }
    // else
    {
        try
        {
            std::vector<Glib::RefPtr<Gio::File> > files;
            res = this->desktop_app_->launch(files);
        }
        catch (const Glib::Error &e)
        {
            res = false;
            error = e.what().raw();
        }
    }

    if (res)
    {
        this->launched_.emit(this->shared_from_this());
    }
    else
    {
        this->launch_failed_.emit(this->shared_from_this());
        g_warning("Failed to launch: %s", error.c_str());
    }
    return res;
}

void App::init_app_kind()
{
    this->kind_ = AppKind::UNKNOWN;

    RETURN_IF_FALSE(this->exec_.length() > 0);
    RETURN_IF_FALSE((bool)this->desktop_app_);

    auto exec_split = str_split(this->exec_, " ");

    if (exec_split.size() == 0)
    {
        return;
    }

    auto file = Gio::File::create_for_path(exec_split[0]);
    if (!file)
    {
        g_warning("create file %s fail.", exec_split[0].c_str());
        return;
    }

    auto exec_name = file->get_basename();

    if (exec_name == "flatpak")
    {
        this->kind_ = AppKind::FLATPAK;
        return;
    }

    auto x_flatpak = this->desktop_app_->get_string("X-Flatpak").raw();
    if (x_flatpak.length() > 0)
    {
        this->kind_ = AppKind::FLATPAK;
        return;
    }

    this->kind_ = AppKind::DESKTOP;
}

typedef struct
{
    GSpawnChildSetupFunc user_setup;
    gpointer user_setup_data;

    char *pid_envvar;
} ChildSetupData;

void App::expand_macro(char macro, GString *exec)
{
    char *expanded = NULL;

    g_return_if_fail(exec != NULL);

    switch (macro)
    {
        case 'i':
            if (this->icon_name_.length() > 0)
            {
                g_string_append(exec, "--icon ");
                expanded = g_shell_quote(this->icon_name_.c_str());
                g_string_append(exec, expanded);
                g_free(expanded);
            }
            break;

        case 'c':
            if (this->locale_name_.length() > 0)
            {
                expanded = g_shell_quote(this->locale_name_.c_str());
                g_string_append(exec, expanded);
                g_free(expanded);
            }
            break;

        case 'k':
            if (this->file_name_.length() > 0)
            {
                expanded = g_shell_quote(this->file_name_.c_str());
                g_string_append(exec, expanded);
                g_free(expanded);
            }
            break;

        case '%':
            g_string_append_c(exec, '%');
            break;
    }
}

bool App::expand_application_parameters(int *argc,
                                        char ***argv,
                                        GError **error)
{
    const char *p = this->exec_.c_str();
    GString *expanded_exec;
    bool res;

    if (this->exec_.length() == 0)
    {
        g_set_error_literal(error,
                            G_IO_ERROR,
                            G_IO_ERROR_FAILED,
                            "Desktop file didn???t specify Exec field");
        return false;
    }

    expanded_exec = g_string_new(NULL);

    while (*p)
    {
        if (p[0] == '%' && p[1] != '\0')
        {
            expand_macro(p[1], expanded_exec);
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

bool App::launch_flatpak(GError **error)
{
    bool completed = false;

    char **argv, **envp;
    int argc;
    gchar *pid_envvar;

    argv = NULL;
    envp = g_get_environ();

    GPid pid;
    GList *iter;

    if (!expand_application_parameters(&argc, &argv, error))
        goto out;

    if (this->file_name_.size() > 0)
    {
        envp = g_environ_setenv(envp,
                                "GIO_LAUNCHED_DESKTOP_FILE",
                                this->file_name_.c_str(),
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

    if (!g_spawn_async(this->path_.c_str(),
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

    completed = true;

out:
    g_strfreev(argv);
    g_strfreev(envp);

    return completed;
}

}  // namespace Kiran
