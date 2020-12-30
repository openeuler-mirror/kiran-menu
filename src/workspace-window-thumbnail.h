/**
 * @file workspace-window-thumbnail.h
 * @brief 工作区预览窗口中的窗口缩略图控件
 * @author songchuanfei <songchuanfei@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved.
 */
#ifndef WORKSPACE_WINDOW_THUMBNAIL_INCLUDE_H
#define WORKSPACE_WINDOW_THUMBNAIL_INCLUDE_H

#include <gtkmm.h>
#include <gtk/gtkx.h>
#include "window-thumbnail-widget.h"
#include "kiran-helper.h"

class WorkspaceWindowThumbnail : public WindowThumbnailWidget
{
public:
    /**
     * @brief 构造函数
     * @param window_   缩略图所属的窗口
     * @param scale_    缩略图显示缩放比例
     */
    WorkspaceWindowThumbnail(KiranWindowPointer &window_, double scale_);
    ~WorkspaceWindowThumbnail();

protected:
    virtual void get_preferred_width_vfunc(int &min_width, int &natural_width) const override;
    virtual bool on_draw(const Cairo::RefPtr<Cairo::Context> &cr) override;

    virtual void on_drag_data_delete (const Glib::RefPtr< Gdk::DragContext >& context) override;
    virtual void on_drag_data_get(const Glib::RefPtr< Gdk::DragContext >& context, Gtk::SelectionData& selection_data, guint info, guint time) override;
    virtual void on_drag_begin (const Glib::RefPtr< Gdk::DragContext >& context) override;

    /**
     * @see KiranThumbnailWidget::draw_thumbnail_image
     */
    virtual bool draw_thumbnail_image(Gtk::Widget *area, const Cairo::RefPtr<Cairo::Context> &cr) override;

    /**
     * @see KiranThumbnailWidget::on_thumbnail_clicked
     */
    virtual void on_thumbnail_clicked() override;

    /**
     * @brief 回调函数：拖放失败时调用，@see signal_drag_failed()
     * @param context 拖放上下文
     * @param result  拖放结果
     * @return 是否处理成功
     */
    virtual bool on_drag_failed(const Glib::RefPtr< Gdk::DragContext >& context, Gtk::DragResult result);

    /**
     * @brief 生成缩放后的窗口缩略图
     * @return 生成成功返回true，失败返回false
     */
    virtual bool generate_thumbnail();

    /**
     * @brief 初始化拖放支持
     */
    void init_drag_and_drop();

private:
    double scale;                           //缩略图缩放比例
    int border_width;                       //鼠标经过时绘制的缩略图边框宽度
    cairo_surface_t *thumbnail_surface;     //窗口缩略图
    int thumbnail_width, thumbnail_height;  //窗口缩略图尺寸

    int window_width, window_height;        //窗口真实大小
    bool show_thumbnail;                    //是否绘制窗口缩略图
};

#endif // WORKSPACE_WINDOW_THUMBNAIL_INCLUDE_H
