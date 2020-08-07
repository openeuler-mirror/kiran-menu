#ifndef KIRANTASKLISTWIDGET_H
#define KIRANTASKLISTWIDGET_H

#include "kiran-tasklist-app-button.h"
#include "kiran-tasklist-app-previewer.h"
#include <mate-panel-applet.h>
#include <vector>
#include <map>
#include "app.h"
#include "app-manager.h"
#include "kiranhelper.h"

class KiranTasklistWidget : public Gtk::Box
{

public:
    KiranTasklistWidget(MatePanelApplet *applet_);
    ~KiranTasklistWidget();

    void add_app_button(const KiranAppPointer &app);
    void remove_app_button(const KiranAppPointer &app);
    KiranAppPointer get_current_active_app();

    KiranTasklistAppButton *find_app_button(const KiranAppPointer &app);

    void on_active_window_changed(KiranWindowPointer previous, KiranWindowPointer active);
    void on_window_opened(KiranWindowPointer window);
    void on_window_closed(KiranWindowPointer window);

    //移动预览窗口到指定的应用按钮，并显示该应用的窗口预览信息
    void move_previewer(KiranTasklistAppButton *target);
    //隐藏预览窗口
    void hide_previewer();

    //根据applet所在面板的排列方向更新应用按钮排列方向
    void update_orientation();

    void reload_app_buttons();

    void toggle_previewer(KiranTasklistAppButton *target);
    void handle_applet_size_change(int size);
    void handle_applet_orient_change(int orient);

protected:


    virtual void get_preferred_width_vfunc(int &min_width, int &natural_width) const override;
    virtual void get_preferred_height_vfunc(int &min_height, int &natural_height) const override;

    virtual void on_size_allocate(Gtk::Allocation &alloction) override;

    virtual void load_applications();

    virtual void on_fixed_apps_added(const Kiran::AppVec &apps);
    virtual void on_fixed_apps_removed(const Kiran::AppVec &apps);
    virtual void on_previewer_window_opened();

private:
    MatePanelApplet *applet;
    std::map<KiranAppPointer, KiranTasklistAppButton*> app_buttons; //任务栏应用按钮列表
    Kiran::AppManager *manager;

    KiranAppPointer active_app;             //当前活动窗口所属app(cached，不一定是最新的)
    KiranAppPreviewer *previewer;           //应用预览窗口
};

#endif // KIRANTASKLISTWIDGET_H
