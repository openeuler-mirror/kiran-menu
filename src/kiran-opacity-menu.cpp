#include "kiran-opacity-menu.h"

bool KiranOpacityMenu::on_draw(const Cairo::RefPtr<Cairo::Context> &cr)
{
    auto window = get_window();

    cr->save();

    //先将窗口背景设置透明，然后再绘制背景和边框，否则圆角周围会出现多余的窗口背景色
    cr->set_operator(Cairo::OPERATOR_CLEAR);
    cr->paint();
    cr->set_operator(Cairo::OPERATOR_SOURCE);
    get_style_context()->render_background(cr, 0, 0,
                                           window->get_width(),
                                           window->get_height());
    get_style_context()->render_frame(cr, 0, 0,
                                      window->get_width(),
                                      window->get_height());
    cr->restore();

    for (auto child: get_children()) {
        propagate_draw(*child, cr);
    }

    return false;
}
