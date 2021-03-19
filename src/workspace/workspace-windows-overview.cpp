#include "workspace-windows-overview.h"
#include "workspace-window-thumbnail.h"
#include "kiran-helper.h"
#include "window_arrange.h"
#include <algorithm>
#include "global.h"
#include "log.h"

#define MIN_VIEW_WIDTH          80          //窗口截图的最小宽度
#define MIN_VIEW_HEIGHT         150         //窗口截图的最小高度

/* FIXME:  能否通过动态获取的方式来拿到窗口缩略图上方的title区域高度 */
#define SNAPSHOT_EXTRA_HEIGHT   44          //窗口截图控件除缩略图部分外的额外高度

WorkspaceWindowsOverview::WorkspaceWindowsOverview():
    old_allocation(-1, -1, 0, 0),
    layout(Gtk::ORIENTATION_VERTICAL),
    row_spacing(15),
    column_spacing(15),
    max_rows(4)
{
    add(layout);

    set_has_window(true);
    set_hexpand(true);
    set_vexpand(true);
    set_halign(Gtk::ALIGN_FILL);
    set_valign(Gtk::ALIGN_FILL);

    layout.set_margin_left(24);
    layout.set_margin_right(24);
    layout.set_margin_top(20);
    layout.set_margin_bottom(20);
    layout.set_spacing(row_spacing);

    add_events(Gdk::BUTTON_PRESS_MASK);
}

WorkspaceWindowsOverview::~WorkspaceWindowsOverview()
{
    if (reload_handler.connected())
        reload_handler.disconnect();
}

void WorkspaceWindowsOverview::set_workspace(KiranWorkspacePointer &workspace_)
{
    workspace = workspace_;
    Glib::signal_idle().connect_once(sigc::mem_fun(*this, &WorkspaceWindowsOverview::reload_thumbnails));
}

void WorkspaceWindowsOverview::set_row_spacing(int spacing)
{
    row_spacing = spacing;
    if (get_realized())
        queue_resize();
}

void WorkspaceWindowsOverview::set_column_spacing(int spacing)
{
    column_spacing = spacing;
    if (get_realized())
        queue_resize();
}

void WorkspaceWindowsOverview::set_max_rows(int max_rows_) 
{
    max_rows = max_rows_;
    if (get_realized())
        queue_resize();
}

void WorkspaceWindowsOverview::clear()
{
    KiranHelper::remove_all_for_container(layout, true);
}

bool WorkspaceWindowsOverview::on_draw(const Cairo::RefPtr<Cairo::Context> &cr)
{
    if (workspace.expired())
        return false;
    return Gtk::EventBox::on_draw(cr);
}


void WorkspaceWindowsOverview::on_size_allocate(Gtk::Allocation &allocation)
{
    Gtk::Allocation child_allocation = allocation;

    set_allocation(allocation);
    if (get_realized()) {
        /* 调整GdkWindow尺寸和位置 */
        get_window()->move_resize(allocation.get_x(),
                                  allocation.get_y(),
                                  allocation.get_width(),
                                  allocation.get_height());
    }

    if (old_allocation.get_width() != allocation.get_width() || old_allocation.get_height() != allocation.get_height()) {
        /* 缩略图区域大小变化时重新排列窗口缩略图 */
        if (reload_handler.connected())
        {
            g_debug("workspace overview allocate, reloading thumbnails already scheduled");
            return;
        }

        g_debug("workspace overview allocate %d x %d, schedule for reloading thumbnails",
                allocation.get_width(),
                allocation.get_height());
        clear();
        reload_handler = Glib::signal_idle().connect(
                sigc::bind_return<bool>(
                    sigc::mem_fun(*this, &WorkspaceWindowsOverview::reload_thumbnails),
                    false));
    }
    child_allocation.set_x(0);
    child_allocation.set_y(0);
    layout.size_allocate(child_allocation);
    old_allocation = allocation;
}

bool WorkspaceWindowsOverview::on_button_press_event(GdkEventButton *event)
{
    /*Hide window when clicked on blank area*/
    get_toplevel()->hide();
    return false;
}

void WorkspaceWindowsOverview::reload_thumbnails()
{
    std::vector<int> width_vector;
    Kiran::WindowVec windows;
    int viewport_width, viewport_height;                    //显示区域的宽度和高度
    int rows = 1;                                           //窗口的显示行数
    int scale_factor = get_scale_factor();

    clear();
    if (workspace.expired()) {
        LOG_WARNING("workspace already expired");
        return;
    }

    if (!get_realized())
        return;

    if (layout.get_allocated_width() <= 1 || layout.get_allocated_height() <= 1)
        return;

    viewport_width = layout.get_allocated_width() - layout.get_margin_left() - layout.get_margin_right();
    viewport_height = layout.get_allocated_height() - layout.get_margin_top() - layout.get_margin_bottom();

    /**
     * 过滤掉需要跳过工作区显示的窗口
     */
    windows = workspace.lock()->get_windows();
    auto iter = std::remove_if(windows.begin(), windows.end(),
                   [](KiranWindowPointer &w) -> bool {
                        return w->is_skip_pager();
                   });

    windows.erase(iter, windows.end());
    if (windows.size() == 0)
        return;

    /*
     * 计算合适的窗口显示行数
     * FIXME: max_rows应当根据屏幕高度动态调整
     */
    rows = calculate_rows(windows, viewport_width, viewport_height);
    LOG_DEBUG("viewport size %d x %d, rows %d",
              viewport_width,
              viewport_height,
              rows);

    /*
     * 将窗口分成rows分组，确保每组窗口的宽度之和近似
     */
    std::vector<std::vector<int16_t>> results;
    for (auto window: windows) {
        width_vector.push_back(WINDOW_WIDTH(window));
    }
    results = arrange_data(width_vector, rows);


    /*
     * 开始计算每行的实际缩放比，然后绘制窗口缩略图
     */
    int index = 0;
    for (auto row: results) {
        int sum = 0;
        double scale, x_scale, y_scale = 1.0;
        int max_height = 0;

        //把窗口按照先宽度，后高度的方式进行排序，值大的在前，只是为了好看^^)
        std::sort(row.begin(), row.end(),[&windows](uint32_t i, uint32_t j){
            auto w1 = windows.at(i);
            auto w2 = windows.at(j);

            if (WINDOW_WIDTH(w1) != WINDOW_WIDTH(w2))
                return WINDOW_WIDTH(w1) >= WINDOW_WIDTH(w2);
            else
                return WINDOW_HEIGHT(w1) >= WINDOW_HEIGHT(w2);
        });

        /*
         * 提取该行窗口的最大高度
         */
        for (auto index: row) {
            auto window = windows.at(index);

            sum += WINDOW_WIDTH(window);
            max_height = std::max(max_height, WINDOW_HEIGHT(window));
        }

        /*
         * 计算每行横向和纵向的实际缩放比(去掉窗口之间的间隔)，取两者的最小值,
         * 确保该行最大高度的窗口缩放后也可以正常显示
         */
        x_scale = (viewport_width - (row.size() - 1) * column_spacing) * scale_factor * 1.0/sum;
        y_scale = (viewport_height - rows * (row_spacing + SNAPSHOT_EXTRA_HEIGHT) + row_spacing) * scale_factor * 1.0/rows/max_height;
        scale = std::min(x_scale, y_scale);

        if (scale > 1.0)
            scale = 1.0;

        LOG_DEBUG("row[%d], real scale %.2lf, x_scale %.2lf, y_scale %.2lf, max_height %d, sum %d\n",
                  index++, scale, x_scale, y_scale, max_height, sum);

        Gtk::Box *row_box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL);
        row_box->set_halign(Gtk::ALIGN_START);
        row_box->set_spacing(column_spacing);
        for (auto index: row) {
            auto thumbnail = Gtk::make_managed<WorkspaceWindowThumbnail>(windows.at(index), scale);
            row_box->pack_start(*thumbnail, Gtk::PACK_SHRINK);
            thumbnail->show_all();
        }
        row_box->show_all();
        layout.pack_start(*row_box, Gtk::PACK_SHRINK);
    }

    layout.show_all();
}

int WorkspaceWindowsOverview::calculate_rows(std::vector<std::shared_ptr<Kiran::Window> > &windows,
                                             int viewport_width,
                                             int viewport_height)
{
    int rows = 1;
    double scale_delta = 0;                                 //每行宽度缩放比和高度缩放比的差值
    int min_width = G_MAXINT, min_height = G_MAXINT, max_height = 0;
    gint64 sum_width = 0;

    /*
     * 遍历窗口，获取窗口宽度和高度的最小值，高度的最大值，以及所有窗口宽度之和
     */
    for (auto window: windows) {
        int width, height;

        width = WINDOW_WIDTH(window);
        height = WINDOW_HEIGHT(window);

        sum_width += width;
        min_width = std::min(min_width, width);
        min_height = std::min(min_height, height);
        max_height = std::max(max_height, height);
    }

    /*
     * 约束:
     * 1、最小宽度和最小高度的窗口经过缩放后尺寸不能小于阈值，确保窗口预览可见
     * 2、最大高度的窗口经过缩放后也在行高的范围内，同时最接近行高
     *
     */
    int scale_factor = get_scale_factor();
    bool found_ok = false;
    do {
        double x_scale, y_scale;

        x_scale = viewport_width * rows * scale_factor * 1.0/sum_width;        //宽度缩放比
        y_scale = (viewport_height - rows * (row_spacing + SNAPSHOT_EXTRA_HEIGHT) + row_spacing) * scale_factor * 1.0/(rows * max_height);    //高度缩放比

        if (x_scale <= y_scale && min_width * x_scale > MIN_VIEW_WIDTH * scale_factor && min_height * x_scale > MIN_VIEW_HEIGHT * scale_factor) {
            found_ok = true;
        }

        /* 随着行数的增加，x_scale逐渐增大， y_scale逐渐减小，当两者最接近时窗口缩略图的显示效果最好。*/
        if (x_scale > y_scale) {
            if (scale_delta < std::abs(x_scale - y_scale) && found_ok)
                rows--;
            break;
        }

        scale_delta = std::abs(x_scale - y_scale);
        rows++;
    } while (rows <= max_rows);

    rows = std::max(rows, 1);
    /* FIXME: 如果按照最大行数依旧无法找到满足条件的缩放比，是否可以考虑滚动??? */
    if (rows > max_rows) {
        LOG_WARNING("No valid rows count found, use max_rows %d", max_rows);
        rows = max_rows;
    }
    return rows;
}