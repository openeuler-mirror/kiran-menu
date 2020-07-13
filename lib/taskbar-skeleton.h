/*
 * @Author       : tangjie02
 * @Date         : 2020-07-09 11:03:30
 * @LastEditors  : tangjie02
 * @LastEditTime : 2020-07-09 14:31:25
 * @Description  : 
 * @FilePath     : /kiran-menu-2.0/lib/taskbar-skeleton.h
 */

#include "lib/app-manager.h"

namespace Kiran
{
class TaskBarSkeleton
{
public:
    TaskBarSkeleton(AppManager *app_manager);
    virtual ~TaskBarSkeleton();

    static TaskBarSkeleton *get_instance() { return instance_; };

    static void global_init(AppManager *app_manager);

    static void global_deinit() { delete instance_; };

    // 将desktop_id加入固定列表，如果dekstop_id不合法, 或者已经在列表中, 则返回false, 否则返回true.
    bool add_fixed_app(const std::string &desktop_id);

    // 从固定列表删除desktop_id，如果dekstop_id不在列表中, 则返回false, 否则返回true
    bool del_fixed_app(const std::string &desktop_id);

    // 查询desktop_id是否在固定列表中，如果未查询到，则返回空指针
    std::shared_ptr<App> lookup_fixed_app(const std::string &desktop_id);

    // 获取固定列表APP
    AppVec get_fixed_apps();

    // 固定APP添加信号
    sigc::signal<void, AppVec> &signal_fixed_app_added() { return this->fixed_app_added_; };

    // 固定APP删除信号
    sigc::signal<void, AppVec> &signal_fixed_app_deleted() { return this->fixed_app_deleted_; };

private:
    void init();

    void desktop_app_changed();

    void app_changed(const Glib::ustring &key);

protected:
    sigc::signal<void, AppVec> fixed_app_added_;
    sigc::signal<void, AppVec> fixed_app_deleted_;

private:
    static TaskBarSkeleton *instance_;

    AppManager *app_manager_;

    Glib::RefPtr<Gio::Settings> settings_;

    std::list<int32_t> fixed_apps_;
};

}  // namespace Kiran