#include "src/start-menu/kiran-app-usage.h"
#include "src/start-menu/kiran-skeleton.h"

struct _KiranAppUsage {
  GObject parent;
  KiranStartMenuS *skeleton;
  GHashTable *app_usages;
};

G_DEFINE_TYPE(KiranAppUsage, kiran_app_usage, G_TYPE_OBJECT);


struct UsageData
{
  gdouble score; 
  long last_seen;
};

/*static int sort_apps_by_usage (gconstpointer a,
                    gconstpointer b,
                    gpointer      data)
{
  KiranAppUsage *self = KIRAN_APP_USAGE(data);
  UsageData *usage_a, *usage_b;
  usage_a = g_hash_table_lookup (self->app_usages, a);
  usage_b = g_hash_table_lookup (self->app_usages, b);
  return usage_b->score - usage_a->score;
}*/

static gboolean hanle_get_nfrequent_apps(KiranStartMenuS *skeleton,
                                          GDBusMethodInvocation *invocation,
                                          int top_n, KiranAppUsage *self) {

  GSList *apps;
  char *appid;
  GHashTableIter iter;

  g_hash_table_iter_init (&iter, self->app_usages);
  apps = NULL;
  while (g_hash_table_iter_next (&iter, (gpointer *) &appid, NULL))
    {
      apps = g_slist_prepend (apps, g_object_ref (appid));
    }

  //apps = g_slist_sort_with_data (apps, sort_apps_by_usage, self);

  //kiran_start_menu_s_complete_get_all_sorted_apps(skeleton, invocation, g_list_f);
  return TRUE;

}

static void load_app_usages(KiranAppUsage *self) {}

static void kiran_app_usage_init(KiranAppUsage *self) {

  load_app_usages(self);

  g_signal_connect(kiran_start_menu_s_get_default(), "handle-get-nfrequent-apps",
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
