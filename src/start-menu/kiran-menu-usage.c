/*
 * @Author       : tangjie02
 * @Date         : 2020-04-09 20:35:20
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-04-10 01:14:43
 * @Description  :
 * @FilePath     : /kiran-menu-backend/src/start-menu/kiran-menu-usage.c
 */
#include "src/start-menu/kiran-menu-usage.h"

#include "src/start-menu/kiran-menu-common.h"
#include "src/utils/helper.h"
#include "src/utils/math_helper.h"

struct _KiranMenuUsage {
  GObject parent;
  GSettings *settings;
  GHashTable *app_usages;
  gchar *focus_desktop_id;
};

G_DEFINE_TYPE(KiranMenuUsage, kiran_menu_usage, G_TYPE_OBJECT);

#define FOCUS_TIME_MIN_SECONDS 2
#define FOCUS_TIME_MAX_SECONDS 1000
#define EPS 1e-8

typedef struct UsageData UsageData;
struct UsageData {
  gdouble score;
  long last_seen;
};

static long get_system_time(void) {
  return g_get_real_time() / G_TIME_SPAN_SECOND;
}

static UsageData *get_usage_for_app(KiranMenuUsage *self,
                                    const char *desktop_id) {
  UsageData *usage;

  GQuark quark = g_quark_from_string(desktop_id);

  usage = g_hash_table_lookup(self->app_usages, GUINT_TO_POINTER(quark));
  if (usage) return usage;

  usage = g_new0(UsageData, 1);
  g_hash_table_insert(self->app_usages, GUINT_TO_POINTER(quark), usage);
  return usage;
}

static gboolean write_usages_to_settings(KiranMenuUsage *self) {
  GVariantBuilder builder;
  g_variant_builder_init(&builder, G_VARIANT_TYPE("a{sv}"));

  GHashTableIter iter;
  gpointer key;
  UsageData *usage;
  g_hash_table_iter_init(&iter, self->app_usages);
  while (g_hash_table_iter_next(&iter, (gpointer *)&key, (gpointer *)&usage)) {
    GVariant *v_usage = g_variant_new("(di)", usage->score, usage->last_seen);
    GQuark quark = GPOINTER_TO_UINT(key);
    const char *desktop_id = g_quark_to_string(quark);
    g_variant_builder_add(&builder, "{sv}", desktop_id,
                          g_variant_new_variant(v_usage));
  }
  g_autoptr(GVariant) frequent_apps = g_variant_builder_end(&builder);
  return g_settings_set_value(self->settings, "frequent-apps", frequent_apps);
}

static gboolean read_usages_from_settings(KiranMenuUsage *self) {
  g_autoptr(GVariant) frequent_apps =
      g_settings_get_value(self->settings, "frequent-apps");

  if (frequent_apps) {
    gsize child_num = g_variant_n_children(frequent_apps);
    for (gsize i = 0; i < child_num; ++i) {
      gchar *desktop_id;
      GVariant *v_usage;
      UsageData *usage = g_new0(UsageData, 1);

      g_variant_get_child(frequent_apps, i, "{sv}", &desktop_id, &v_usage);
      v_usage = g_variant_get_variant(v_usage);
      g_variant_get(v_usage, "(di)", &(usage->score), &(usage->last_seen));

      GQuark quark = g_quark_from_string(desktop_id);
      if (!g_hash_table_insert(self->app_usages, GUINT_TO_POINTER(quark),
                               usage)) {
        return FALSE;
      }
    }
  }
  return TRUE;
}

static void increment_usage_by_latest_use_time(KiranMenuUsage *self,
                                               char *desktop_id, long time) {
  UsageData *usage;
  usage = get_usage_for_app(self, desktop_id);
  if (time >= FOCUS_TIME_MIN_SECONDS) {
    time = (time > FOCUS_TIME_MAX_SECONDS) ? FOCUS_TIME_MAX_SECONDS : time;
    usage->score += kiran_math_log10(time);
  }
}

static int sort_apps_by_usage(gconstpointer a, gconstpointer b, gpointer data) {
  GQuark qa = g_quark_from_string((const gchar *)a);
  GQuark qb = g_quark_from_string((const gchar *)b);
  KiranMenuUsage *self = KIRAN_MENU_USAGE(data);

  UsageData *usage_a =
      (UsageData *)g_hash_table_lookup(self->app_usages, GUINT_TO_POINTER(qa));
  UsageData *usage_b =
      (UsageData *)g_hash_table_lookup(self->app_usages, GUINT_TO_POINTER(qb));

  gdouble a_score = (usage_a == NULL) ? 0 : usage_a->score;
  gdouble b_score = (usage_b == NULL) ? 0 : usage_b->score;
  if (ABS(a_score - b_score) < EPS) return 0;
  return a_score > b_score ? -1 : 1;
}

gboolean kiran_menu_usage_focus_app(KiranMenuUsage *self,
                                    const char *desktop_id) {
  g_return_val_if_fail(desktop_id != NULL, FALSE);

  if (g_strcmp0(self->focus_desktop_id, desktop_id) == 0) {
    return TRUE;
  }

  UsageData *usage_data = NULL;
  long cur_system_time = get_system_time();

  if (self->focus_desktop_id != NULL) {
    usage_data = get_usage_for_app(self, self->focus_desktop_id);
    increment_usage_by_latest_use_time(self, self->focus_desktop_id,
                                       cur_system_time - usage_data->last_seen);
    g_clear_pointer(&self->focus_desktop_id, g_free);
  }

  self->focus_desktop_id = g_strdup(desktop_id);
  usage_data = get_usage_for_app(self, self->focus_desktop_id);
  usage_data->last_seen = cur_system_time;

  // TODO: need to optimize in idle time call
  write_usages_to_settings(self);
  return TRUE;
}

GList *kiran_menu_usage_get_nfrequent_apps(KiranMenuUsage *self, gint top_n) {
  GHashTableIter iter;
  gpointer key;
  UsageData *value;
  GList *apps = NULL;

  g_hash_table_iter_init(&iter, self->app_usages);
  while (g_hash_table_iter_next(&iter, (gpointer)&key, (gpointer)&value)) {
    if (value->score > EPS) {
      GQuark quark = GPOINTER_TO_UINT(key);
      const char *desktop_id = g_quark_to_string(quark);
      apps = g_list_append(apps, g_strdup(desktop_id));
    }
  }
  apps = g_list_sort_with_data(apps, sort_apps_by_usage, self);
  return list_remain_headn(apps, top_n, g_free);
}

gboolean kiran_menu_usage_reset(KiranMenuUsage *self) {
  g_clear_pointer(&self->app_usages, g_hash_table_unref);
  g_clear_pointer(&self->focus_desktop_id, g_free);
  write_usages_to_settings(self);
  return TRUE;
}

static void kiran_menu_usage_init(KiranMenuUsage *self) {
  self->settings = g_settings_new(KIRAN_MENU_SCHEMA);
  self->app_usages = g_hash_table_new_full(NULL, NULL, NULL, g_free);
  read_usages_from_settings(self);
}

static void kiran_menu_usage_dispose(GObject *object) {
  KiranMenuUsage *menu_usage = KIRAN_MENU_USAGE(object);

  g_clear_pointer(&menu_usage->settings, g_object_unref);
  g_clear_pointer(&menu_usage->app_usages, g_hash_table_unref);
  g_clear_pointer(&menu_usage->focus_desktop_id, g_free);

  G_OBJECT_CLASS(kiran_menu_usage_parent_class)->dispose(object);
}

static void kiran_menu_usage_class_init(KiranMenuUsageClass *klass) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

  gobject_class->dispose = kiran_menu_usage_dispose;
}

/**
 * kiran_menu_usage_get_default:
 *
 * Return Value: (transfer none): The global #KiranMenuUsage instance
 */
KiranMenuUsage *kiran_menu_usage_get_new() {
  return g_object_new(KIRAN_TYPE_MENU_USAGE, NULL);
}
