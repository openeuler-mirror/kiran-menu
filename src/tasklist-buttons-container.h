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

class TasklistButtonsContainer : public Gtk::ScrolledWindow
{

public:
    TasklistButtonsContainer(MatePanelApplet *applet_);
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
    Gtk::Orientation get_orientation();

    void handle_applet_size_change();
    int get_applet_size() const;

    void reload_app_buttons();

    void toggle_previewer(TasklistAppButton *target);


    void move_to_next_page();
    void move_to_previous_page();
    bool has_previous_page();
    bool has_next_page();

    sigc::signal<void> signal_page_changed();

protected:
    virtual void get_preferred_width_vfunc(int &min_width, int &natural_width) const override;
    virtual void get_preferred_height_vfunc(int &min_height, int &natural_height) const override;

    virtual void on_size_allocate(Gtk::Allocation &alloction) override;
    virtual void on_map() override;

    virtual void load_applications();

    virtual void on_fixed_apps_added(const Kiran::AppVec &apps);
    virtual void on_fixed_apps_removed(const Kiran::AppVec &apps);
    virtual void on_previewer_window_opened();

    virtual void init_ui();

private:
    MatePanelApplet *applet;
    std::map<KiranAppPointer, TasklistAppButton*> app_buttons; //任务栏应用按钮列表
    Gtk::Box box;

    KiranAppPointer active_app;             //当前活动窗口所属app(cached，不一定是最新的)
    TasklistAppPreviewer *previewer;        //应用预览窗口

    int signal_applet_size_changed;
    sigc::signal<void> m_signal_page_changed;
};

#endif // TASKLIST_BUTTONS_CONTAINER_H
