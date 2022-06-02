/*
 * @Author       : tangjie02
 * @Date         : 2020-04-08 14:23:14
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-06-05 09:59:55
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
    /**
     * @description: 通过关键词进行检索, 会跟.desktop文件的name/localename/comment字段进行字符串匹配,
     * 如果关键词为其中任何一个字段的子串, 则匹配成功. 最后返回所有匹配成功的App.
     * @param {const std::string &} keyword 检索的关键词
     * @param {bool} ignore_case 忽略大小写
     * @return: 返回匹配成功APP列表,  如果没有匹配成功列表或者出现错误，则返回空列表,
     * @author: tangjie02
     */
    virtual AppVec search_app(const std::string &keyword, bool ignore_case = false) = 0;

    /**
     * @description: 将desktop_id加入收藏列表.
     * @param {const std::string &} desktop_id 收藏的desktop_id
     * @return: 如果dekstop_id不合法, 或者已经在收藏列表中, 则返回false, 否则返回true.
     * @author: tangjie02
     */
    virtual bool add_favorite_app(const std::string &desktop_id) = 0;

    /**
     * @description: 从收藏列表删除desktop_id.
     * @param {const std::string &} desktop_id 删除的desktop_id
     * @return: 如果dekstop_id不在收藏列表中, 则返回false, 否则返回true.
     * @author: tangjie02
     */
    virtual bool del_favorite_app(const std::string &desktop_id) = 0;

    /**
     * @description: 查询desktop_id是否在收藏列表中
     * @param {const std::string &} desktop_id 查询的desktop_id
     * @return: 返回查询到的app，如果未查询到，则返回nullptr.
     * @author: tangjie02
     */
    virtual std::shared_ptr<App> lookup_favorite_app(const std::string &desktop_id) = 0;

    /**
     * @description: 获取收藏列表.
     * @return: 返回收藏APP列表.
     * @author: tangjie02
     */
    virtual AppVec get_favorite_apps() = 0;

    /**
     * @description: 将desktop_id添加到category分类中.
     * @param {const std::string &} category 选择的分类
     * @param {const std::string &} desktop_id 添加的desktop_id
     * @return: 如果desktop_id不存在或者添加分类错误, 则返回false, 否则返回true.
     * @author: tangjie02
     */
    virtual bool add_category_app(const std::string &category_name, const std::string &desktop_id) = 0;

    /**
     * @description: 将desktop_id从category分类中删除.
     * @param {const std::string &} category 选择的分类
     * @param {const std::string &} desktop_id 删除的desktop_id
     * @return: 如果desktop_id不存在或者删除分类错误, 则返回false, 否则返回true.
     * @author: tangjie02
     */
    virtual bool del_category_app(const std::string &category_name, const std::string &desktop_id) = 0;

    /**
     * @description: 获取所有分类的名字
     * @return: 
     * @author: tangjie02
     */
    virtual std::vector<std::string> get_category_names() = 0;

    /**
     * @description: 获取category分类中的所有App.
     * @param {const std::string &} category_name 选择的分类
     * @return: 
     * g_object_unref)进行释放.
     * @author: tangjie02
     */
    virtual AppVec get_category_apps(const std::string &category_name) = 0;

    /**
     * @description: 获取所有分类的App.
     * @return: 
     * @author: tangjie02
     */
    virtual std::map<std::string, AppVec> get_all_category_apps() = 0;

    /**
     * @description: 获取使用频率最高的top_n个app
     * @param {gint} top_n 见函数说明.
     * @return: 如果top_n超过所有app的数量或者等于-1,则返回所有App.
     * @author: tangjie02
     */
    virtual AppVec get_nfrequent_apps(gint top_n) = 0;

    /**
     * @description: 重置频繁使用的APP列表.该操作会将所有APP的分数清0
     * @return:
     * @author: tangjie02
     */
    virtual void reset_frequent_apps() = 0;

    /**
     * @description: 获取最新安装的top_n个app, 返回这些app的App对象.
     * @return: 如果top_n超过所有app的数量或者等于-1,则返回所有App.
     * @author: tangjie02
     */
    virtual AppVec get_nnew_apps(gint top_n) = 0;

    /**
     * @description: 获取所有已注册且可在当前系统显示的desktop_id列表,该列表已通过.desktop文件的name字段进行排序.
     * @return: 
     * @author: tangjie02
     */
    virtual AppVec get_all_sorted_apps() = 0;
};
}  // namespace Kiran