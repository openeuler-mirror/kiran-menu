#include "src/start-menu/kiran-app-usage.h"

#include "src/start-menu/kiran-skeleton.h"
#include "src/utils/math_helper.h"

struct _KiranAppUsage {
  GObject parent;
  KiranStartMenuS *skeleton;
  GHashTable *app_usages;
  gchar *cur_focus_app;
};

G_DEFINE_TYPE(KiranAppUsage, kiran_app_usage, G_TYPE_OBJECT);

#define FOCUS_TIME_MIN_SECONDS 3
#define FOCUS_TIME_MAX_SECONDS 1000

typedef struct UsageData UsageData;
struct UsageData {
  gdouble score;
  long last_seen;
};

static long get_system_time (void)
{
  return g_get_real_time () / G_TIME_SPAN_SECOND;
}

static UsageData *get_usage_for_app(KiranAppUsage *self, char *app_id) {
  UsageData *usage;

  usage = g_hash_table_lookup(self->app_usages, app_id);
  if (usage) return usage;

  usage = g_new0(UsageData, 1);
  g_hash_table_insert(self->app_usages, g_strdup(app_id), usage);
  return usage;
}

static void write_app_usages(KiranAppUsage *self) {
  GVariantBuilder builder;
  g_variant_builder_init(&builder, G_VARIANT_TYPE("a(sdi)"));

  GHashTableIter iter;
  gchar *desktop_file;
  UsageData *usage;
  g_hash_table_iter_init (&iter, self->app_usages);
  while (g_hash_table_iter_next (&iter, (gpointer *)&desktop_file, (gpointer *) &usage))
  {
    g_variant_builder_add(&builder, "(sdi)", desktop_file, usage->score, usage->last_seen);
  }
  kiran_start_menu_s_set_frequent_apps(self->skeleton, g_variant_builder_end(&builder));
}

static void load_app_usages(KiranAppUsage *self) {
  GVariant *frequent_apps = kiran_start_menu_s_get_frequent_apps(self->skeleton);
  if (frequent_apps) {
    gsize child_num = g_variant_n_children(frequent_apps);
    for (gsize i = 0; i < child_num; ++i) {
      gchar *desktop_file;
      UsageData *usage = g_new0(UsageData, 1);
      g_variant_get_child(frequent_apps, i, "(sdi)", &desktop_file,
                          &(usage->score), &(usage->last_seen));

      g_hash_table_insert(self->app_usages, g_strdup(desktop_file), usage);
    }
  }
}

static void increment_usage_by_latest_use_time(KiranAppUsage *self,
                                               char *app_id, long time) {
  UsageData *usage;
  usage = get_usage_for_app(self, app_id);
  if (time >= FOCUS_TIME_MIN_SECONDS) {
    time = (time > FOCUS_TIME_MAX_SECONDS) ? FOCUS_TIME_MAX_SECONDS : time;
    usage->score += kiran_math_log10(time);
  }
}

static int sort_apps_by_usage(gconstpointer a, gconstpointer b, gpointer data) {
  KiranAppUsage *self = KIRAN_APP_USAGE(data);
  UsageData *usage_a, *usage_b;
  usage_a = (UsageData *)g_hash_table_lookup(self->app_usages, *(gchar **)a);
  usage_b = (UsageData *)g_hash_table_lookup(self->app_usages, *(gchar **)b);
  gdouble a_score = (usage_a == NULL) ? 0 : usage_a->score;
  gdouble b_score = (usage_b == NULL) ? 0 : usage_b->score;
  return b_score - a_score;
}

static gboolean handle_focus_app(KiranStartMenuS *skeleton,
                                 GDBusMethodInvocation *invocation,
                                 char *desktop_file, KiranAppUsage *self) {

  if (g_strcmp0(self->cur_focus_app, desktop_file) == 0)
  {
    return TRUE;
  }
  UsageData *usage_data = NULL;
  long cur_system_time = get_system_time();
  if (self->cur_focus_app != NULL)
  {
    usage_data = get_usage_for_app(self, self->cur_focus_app);
    increment_usage_by_latest_use_time(self, self->cur_focus_app, cur_system_time - usage_data->last_seen);
    g_free(self->cur_focus_app);
  }
  self->cur_focus_app = g_strdup(desktop_file);
  usage_data = get_usage_for_app(self, self->cur_focus_app);
  usage_data->last_seen = cur_system_time;
  // TODO: need to optimize in idle time call
  write_app_usages(self);
  return TRUE;
}

static gboolean hanle_get_nfrequent_apps(KiranStartMenuS *skeleton,
                                         GDBusMethodInvocation *invocation,
                                         int top_n, KiranAppUsage *self) {
  GArray *apps = g_array_new(FALSE, FALSE, sizeof(gchar *));
  GList *registed_apps = g_app_info_get_all();
  for (GList *l = registed_apps; l != NULL; l = l->next) {
    GAppInfo *info = l->data;
    const char *id = g_app_info_get_id(info);
    char *dup_id = g_strdup(id);
    g_array_append_val(apps, dup_id);
  }
  g_array_sort_with_data(apps, sort_apps_by_usage, self);
  if (top_n > apps->len) top_n = apps->len;
  g_array_remove_range(apps, top_n, apps->len - top_n);
  gchar *null = NULL;
  g_array_append_val(apps, null);
  kiran_start_menu_s_complete_get_nfrequent_apps(
      skeleton, invocation, (const gchar *const *)g_array_free(apps, FALSE));
  return TRUE;
}

static void kiran_app_usage_init(KiranAppUsage *self) {
  self->app_usages = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
  self->skeleton = kiran_start_menu_s_get_default();
  load_app_usages(self);

  g_signal_connect(self->skeleton, "handle-focus-app",
                   G_CALLBACK(handle_focus_app), self);
  g_signal_connect(self->skeleton,
                   "handle-get-nfrequent-apps",
                   G_CALLBACK(hanle_get_nfrequent_apps), self);
}

static void kiran_app_usage_finalize(GObject *object) {
  G_OBJECT_CLASS(kiran_app_usage_parent_class)->finalize(object);
}

static void kiran_app_usage_class_init(KiranAppUsageClass *klass) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

  gobject_class->finalize = kiran_app_usage_finalize;
}

/**
 * kiran_app_usage_get_default:
 *
 * Return Value: (transfer none): The global #KiranAppUsage instance
 */
KiranAppUsage *kiran_app_usage_get_new(KiranStartMenuS *skeleton) {
  return g_object_new(KIRAN_TYPE_APP_USAGE, NULL);
}
