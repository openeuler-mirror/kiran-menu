#ifndef TASKLIST_BUTTONS_CONTAINER_H
#define TASKLIST_BUTTONS_CONTAINER_H

#include "tasklist-app-button.h"
#include "tasklist-app-previewer.h"
#include <mate-panel-applet.h>
#include <vector>
#include <map>
#include "app.h"
#include "app-manager.h"
#include "kiran-helper.h"

class TasklistButtonsContainer : public Gtk::Layout
{

public:
    TasklistButtonsContainer(MatePanelApplet *applet_, int child_spacing = 4);
    ~TasklistButtonsContainer() override;

    void add_app_button(const KiranAppPointer &app);
    void remove_app_button(const KiranAppPointer &app);
    KiranAppPointer get_current_active_app();

    TasklistAppButton *find_app_button(const KiranAppPointer &app);

    void on_active_window_changed(KiranWindowPointer previous, KiranWindowPointer active);
    void on_window_opened(KiranWindowPointer window);
    void on_window_closed(KiranWindowPointer window);

    //移动预览窗口到指定的应用按钮，并显示该应用的窗口预览信息
    void move_previewer(TasklistAppButton *target);
    //隐藏预览窗口
    void hide_previewer();
    Gtk::PositionType get_previewer_position();

    //根据applet所在面板的排列方向更新应用按钮排列方向
    void update_orientation();
    Gtk::Orientation get_orientation() const;

    void handle_applet_size_change();
    int get_applet_size() const;

    void reload_app_buttons();

    void toggle_previewer(TasklistAppButton *target);


    void move_to_next_page();
    void move_to_previous_page();
    bool has_previous_page();
    bool has_next_page();

    sigc::signal<void> signal_page_changed();

    Glib::RefPtr<Gtk::Adjustment> get_adjustment();

protected:
    virtual void get_preferred_width_vfunc(int &min_width, int &natural_width) const override;
    virtual void get_preferred_height_vfunc(int &min_height, int &natural_height) const override;

    virtual void on_size_allocate(Gtk::Allocation &allocation) override;
    virtual void on_add(Gtk::Widget *child) override;
    virtual void on_remove(Gtk::Widget *child) override;
    virtual void on_map() override;
    virtual void on_realize() override;

    virtual void load_applications();

    virtual void on_fixed_apps_added(const Kiran::AppVec &apps);
    virtual void on_fixed_apps_removed(const Kiran::AppVec &apps);
    virtual void on_previewer_window_opened();

    /**
     * @brief 确保指定的应用按钮在任务栏上可见。任务栏上空间不足时，通过滚动来保证可见
     * @param button    需要可见的应用按钮
     */
    virtual void switch_to_page_of_button(TasklistAppButton *button);
    virtual void ensure_active_app_button_visible();

    virtual void init_ui();

private:
    MatePanelApplet *applet;
    std::map<KiranAppPointer, TasklistAppButton*> app_buttons; //任务栏应用按钮列表

    KiranAppPointer active_app;             //当前活动窗口所属app(cached，不一定是最新的)
    TasklistAppPreviewer *previewer;        //应用预览窗口

    int signal_applet_size_changed;
    sigc::signal<void> m_signal_page_changed;
    int child_spacing;                      //应用按钮间隔
    int n_child_page;                       //可视区域内的应用按钮个数
    Gtk::Orientation orient;                //应用按钮排列方向
};

#endif // TASKLIST_BUTTONS_CONTAINER_H
