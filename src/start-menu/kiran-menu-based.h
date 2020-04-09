/*
 * @Author       : tangjie02
 * @Date         : 2020-04-08 14:23:14
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-04-10 01:16:20
 * @Description  :
 * @FilePath     : /kiran-menu-backend/src/start-menu/kiran-menu-based.h
 */
#pragma once

#include <gio/gio.h>

G_BEGIN_DECLS

#define KIRAN_TYPE_MENU_BASED (kiran_menu_based_get_type())
G_DECLARE_INTERFACE(KiranMenuBased, kiran_menu_based, KIRAN, MENU_BASED,
                    GObject);

typedef struct _KiranMenuBasedInterface KiranMenuBasedInterface;

struct _KiranMenuBasedInterface {
  GTypeInterface parent;

  GList *(*impl_search_app)(KiranMenuBased *self, const char *keyword);

  gboolean (*impl_add_favorite_app)(KiranMenuBased *self,
                                    const char *desktop_id);

  gboolean (*impl_del_favorite_app)(KiranMenuBased *self,
                                    const char *desktop_id);

  GList *(*impl_get_favorite_apps)(KiranMenuBased *self);

  gboolean (*impl_add_category_app)(KiranMenuBased *self, const char *category,
                                    const char *desktop_id);

  gboolean (*impl_del_category_app)(KiranMenuBased *self, const char *category,
                                    const char *desktop_id);

  GList *(*impl_get_category_apps)(KiranMenuBased *self, const char *category);

  GHashTable *(*impl_get_all_category_apps)(KiranMenuBased *self);

  GList *(*impl_get_nfrequent_apps)(KiranMenuBased *self, gint top_n);

  void (*impl_reset_frequent_apps)(KiranMenuBased *self);

  GList *(*impl_get_nnew_apps)(KiranMenuBased *self, gint top_n);

  GList *(*impl_get_all_sorted_apps)(KiranMenuBased *self);
};

/**
 * @description: 创建一个KiranMenuSkeleton对象.
 * @return:
 * @author: tangjie02
 */
KiranMenuBased *kiran_menu_based_skeleton_new();

/**
 * @description: 通过关键词进行检索,
 * 会跟.desktop文件的name/localename/comment字段进行字符串匹配,
 * 如果关键词为其中任何一个字段的子串, 则匹配成功.
 * 最后返回所有匹配成功的KiranApp.
 * @param {KiranMenuBased*} self KiranMenuSkeleton对象
 * @param {const char*} keyword 检索的关键词
 * @return:
 * 返回匹配成功链表, 链表元素类型为KiranApp*,
 * 如果没有匹配成功列表或者出现错误，则返回NULL,
 * 调用者需要通过g_list_free_full(return_val, g_object_unref)进行释放.
 * @author: tangjie02
 */
GList *kiran_menu_based_search_app(KiranMenuBased *self, const char *keyword);

/**
 * @description: 将desktop_id加入收藏列表.
 * @param {KiranMenuBased*} self KiranMenuSkeleton对象
 * @param {const char*} desktop_id 收藏的desktop_id
 * @return: 如果dekstop_id不合法, 或者已经在收藏列表中, 则返回FALSE,
 * 否则返回TRUE.
 * @author: tangjie02
 */
gboolean kiran_menu_based_add_favorite_app(KiranMenuBased *self,
                                           const char *desktop_id);

/**
 * @description: 从收藏列表删除desktop_id.
 * @param {KiranMenuBased*} self KiranMenuSkeleton对象
 * @param {const char*} desktop_id 删除的desktop_id
 * @return: 如果dekstop_id不在收藏列表中, 则返回FALSE, 否则返回TRUE.
 * @author: tangjie02
 */
gboolean kiran_menu_based_del_favorite_app(KiranMenuBased *self,
                                           const char *desktop_id);

/**
 * @description: 获取收藏列表.
 * @param {KiranMenuBased*} self KiranMenuSkeleton对象
 * @return:
 * 返回收藏列表, 链表元素类型为KiranApp*,
 * 调用者需要通过g_list_free_full(return_val,g_object_unref)进行释放.
 * @author: tangjie02
 */
GList *kiran_menu_based_get_favorite_apps(KiranMenuBased *self);

/**
 * @description: 将desktop_id添加到category分类中.
 * @param {KiranMenuBased*} self KiranMenuSkeleton对象
 * @param {const char*} category 选择的分类
 * @param {const char*} desktop_id 添加的desktop_id
 * @return: 如果desktop_id不存在或者添加分类错误, 则返回FALSE, 否则返回TRUE.
 * @author: tangjie02
 */
gboolean kiran_menu_based_add_category_app(KiranMenuBased *self,
                                           const char *category,
                                           const char *desktop_id);

/**
 * @description: 将desktop_id从category分类中删除.
 * @param {KiranMenuBased*} self KiranMenuSkeleton对象
 * @param {const char*} category 选择的分类
 * @param {const char*} desktop_id 删除的desktop_id
 * @return: 如果desktop_id不存在或者删除分类错误, 则返回FALSE, 否则返回TRUE.
 * @author: tangjie02
 */
gboolean kiran_menu_based_del_category_app(KiranMenuBased *self,
                                           const char *category,
                                           const char *desktop_id);

/**
 * @description: 获取category分类中的所有KiranApp.
 * @param {KiranMenuBased*} self KiranMenuSkeleton对象
 * @param {const char*} category 选择的分类
 * @return: 链表元素类型为KiranApp*, 调用者需要通过g_list_free_full(return_val,
 * g_object_unref)进行释放.
 * @author: tangjie02
 */
GList *kiran_menu_based_get_category_apps(KiranMenuBased *self,
                                          const char *category);

/**
 * @description: 获取所有分类的KiranApp.
 * @param {KiranMenuBased*} self KiranMenuSkeleton对象
 * @return: GHashTable的key为分类字符串, value为GList*,
 * GList中每个元素类型为KiranApp*,
 * 调用者需要通过g_hash_table_unref(return_val)进行释放.
 * @author: tangjie02
 */
GHashTable *kiran_menu_based_get_all_category_apps(KiranMenuBased *self);

/**
 * @description: 获取使用频率最高的top_n个app, 返回app的KiranApp对象.
 * @param {KiranMenuBased*} self KiranMenuSkeleton对象
 * @param {gint} top_n 见函数说明.
 * @return:
 * 链表元素类型为KiranApp*. 如果top_n超过所有app的数量或者等于-1,
 * 则返回所有app的KiranApp.
 * 返回值通过g_list_free_full(return_val,g_object_unref)进行释放.
 * @author: tangjie02
 */
GList *kiran_menu_based_get_nfrequent_apps(KiranMenuBased *self, gint top_n);

/**
 * @description: 重置频繁使用的APP列表.该操作会将所有APP的分数清0
 * @param {KiranMenuBased*} self KiranMenuSkeleton对象
 * @return:
 * @author: tangjie02
 */
void kiran_menu_based_reset_frequent_apps(KiranMenuBased *self);

/**
 * @description: 获取最新安装的top_n个app, 返回这些app的KiranApp对象.
 * @param {KiranMenuBased*} self KiranMenuSkeleton对象
 * @return: 链表元素类型为KiranApp*.如果top_n超过所有app的数量或者等于-1,
 * 则返回所有app的KiranApp.返回值通过g_list_free_full(return_val,g_object_unref)进行释放.
 * @author: tangjie02
 */
GList *kiran_menu_based_get_nnew_apps(KiranMenuBased *self, gint top_n);

/**
 * @description: 获取所有已注册且可在当前系统显示的desktop_id列表,
 * 该列表已通过.desktop文件的name字段进行排序.
 * @param {KiranMenuBased*} self KiranMenuSkeleton对象
 * @return: 链表元素类型为KiranApp*.
 * 返回值通过g_list_free_full(return_val,g_object_unref)进行释放.
 * @author: tangjie02
 */
GList *kiran_menu_based_get_all_sorted_apps(KiranMenuBased *self);

G_END_DECLS