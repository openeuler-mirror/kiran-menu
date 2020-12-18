#ifndef WORKSPACE_APPLET_WINDOW_INCLUDE_H
#define WORKSPACE_APPLET_WINDOW_INCLUDE_H

#include <gtkmm.h>
#include "workspace-thumbnail.h"
#include "workspace-windows-overview.h"
#include <mate-panel-applet.h>

class WorkspaceAppletWindow : public Gtk::Window
{
public:
    WorkspaceAppletWindow();

protected:

    virtual void on_realize() override;
    virtual bool on_draw(const Cairo::RefPtr<Cairo::Context> &cr) override;
    virtual bool on_key_press_event(GdkEventKey *key) override;
    virtual void on_map() override;
    virtual bool on_map_event(GdkEventAny *event) override;
    virtual void on_unmap() override;

    /**
     * @brief 回调函数: 新的工作区创建后调用
     * @param workspace  新创建的工作区
     */
    virtual void on_workspace_created(std::shared_ptr<Kiran::Workspace> workspace);

    /**
     * @brief 回调函数: 工作区被删除后调用
     * @param workspace  被删除的工作区
     */
    virtual void on_workspace_destroyed(std::shared_ptr<Kiran::Workspace> workspace);

    void update_ui();
    void update_workspace(int workspace_num);
    void set_on_all_workspaces();

    void resize_and_reposition();

private:
    MatePanelApplet *applet;
    Glib::RefPtr<Gtk::Builder> builder;

    Gtk::Box *left_layout, *content_layout;
    Gtk::Box *right_layout;
    Gtk::Button *add_button;                        /* 创建工作区按钮 */
    WorkspaceWindowsOverview overview;


    std::map<int, WorkspaceThumbnail*> ws_list;     /* 工作区编号到工作区缩略图控件的映射表 */
    int selected_workspace;                         /* 当前选择显示的工作区编号 */
};

#endif // WORKSPACE_APPLET_WINDOW_INCLUDE_H
