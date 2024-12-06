/**
 * Copyright (c) 2020 ~ 2021 KylinSec Co., Ltd. 
 * kiran-cc-daemon is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2. 
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2 
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, 
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, 
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.  
 * See the Mulan PSL v2 for more details.  
 * 
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 */

#include "lib/app-manager.h"

namespace Kiran
{
class TaskBarSkeleton
{
public:
    typedef enum
    {
        POLICY_SHOW_ALL,              /* 显示所有工作区的应用窗口 */
        POLICY_SHOW_ACTIVE_WORKSPACE, /* 仅显示当前工作区的应用窗口 */
        POLICY_INVALID
    } AppShowPolicy;

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

    AppShowPolicy get_app_show_policy();

    // 固定APP添加信号
    sigc::signal<void, AppVec> &signal_fixed_app_added() { return this->fixed_app_added_; };

    // 固定APP删除信号
    sigc::signal<void, AppVec> &signal_fixed_app_deleted() { return this->fixed_app_deleted_; };

    /* 应用按钮显示策略变化信号 */
    sigc::signal<void> &signal_app_show_policy_changed() { return this->app_show_policy_changed_; }

private:
    void init();

    void desktop_app_changed();

    void app_changed(const Glib::ustring &key);

protected:
    sigc::signal<void, AppVec> fixed_app_added_;
    sigc::signal<void, AppVec> fixed_app_deleted_;
    sigc::signal<void> app_show_policy_changed_;

private:
    static TaskBarSkeleton *instance_;

    AppManager *app_manager_;

    Glib::RefPtr<Gio::Settings> settings_;

    std::list<int32_t> fixed_apps_;
};

}  // namespace Kiran