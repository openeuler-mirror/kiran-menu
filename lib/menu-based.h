/*
 * @Author       : tangjie02
 * @Date         : 2020-04-08 14:23:14
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-10 16:32:41
 * @Description  :
 * @FilePath     : /kiran-menu-2.0/lib/menu-based.h
 */
#pragma once

#include "lib/app.h"

namespace Kiran
{
class MenuBased
{
   public:
    // 通过keyword字段进行检索, 会跟desktop文件的name/localename/comment字段进行字符串匹配, 最重返回所有匹配成功的App。如果ignore_case设置为true，则忽略大小写匹配。
    virtual AppVec search_app(const std::string &keyword, bool ignore_case = false) = 0;

    // 将desktop_id加入收藏列表，如果dekstop_id不合法, 或者已经在收藏列表中, 则返回false, 否则返回true.
    virtual bool add_favorite_app(const std::string &desktop_id) = 0;

    // 从收藏列表删除desktop_id，如果dekstop_id不在收藏列表中, 则返回false, 否则返回true
    virtual bool del_favorite_app(const std::string &desktop_id) = 0;

    // 查询desktop_id是否在收藏列表中，如果未查询到，则返回空指针
    virtual std::shared_ptr<App> lookup_favorite_app(const std::string &desktop_id) = 0;

    // 获取收藏列表
    virtual AppVec get_favorite_apps() = 0;

    // 将desktop_id添加到category分类中，如果desktop_id不存在或者添加分类错误, 则返回false, 否则返回true
    virtual bool add_category_app(const std::string &category_name, const std::string &desktop_id) = 0;

    // 将desktop_id从category分类中删除，如果desktop_id不存在或者删除分类错误, 则返回false, 否则返回true
    virtual bool del_category_app(const std::string &category_name, const std::string &desktop_id) = 0;

    // 获取所有分类的名字
    virtual std::vector<std::string> get_category_names() = 0;

    // 获取category分类中的所有App.
    virtual AppVec get_category_apps(const std::string &category_name) = 0;

    // 获取所有分类的App.
    virtual std::map<std::string, AppVec> get_all_category_apps() = 0;

    // 获取使用频率最高的top_n个app，如果top_n超过所有app的数量或者等于-1，则返回所有App。
    virtual AppVec get_nfrequent_apps(gint top_n) = 0;

    // 重置频繁使用的APP列表.该操作会将所有APP的分数清0
    virtual void reset_frequent_apps() = 0;

    // 获取最新安装的top_n个app, 返回这些app的App对象。如果top_n超过所有app的数量或者等于-1,则返回所有App
    virtual AppVec get_nnew_apps(gint top_n) = 0;

    // 获取所有已注册且可在当前系统显示的desktop_id列表，该列表已通过desktop文件的name字段进行排序
    virtual AppVec get_all_sorted_apps() = 0;

    //  获取当前激活窗口
    virtual std::shared_ptr<Window> get_active_window() = 0;

    // 获取正在运行的应用
    virtual AppVec get_running_apps() = 0;
};
}  // namespace Kiran