#include "menu-list-item-widget.h"
#include <iostream>


MenuListItemWidget::MenuListItemWidget(const std::string &icon_resource,
                                     const std::string &text,
                                     Gtk::Orientation orient,
                                     int icon_size):
    Glib::ObjectBase("KiranMenuListItem"),
    box(orient),
    space_property(*this, "icon-spacing", 0),
    max_width_property(*this, "max-width", G_MAXINT32),
    max_height_property(*this, "max-height", G_MAXINT32)
{
    init_ui();
    set_text(text);
    set_icon(icon_resource, icon_size);

    on_orient_changed();
}

MenuListItemWidget::MenuListItemWidget(int icon_size, Gtk::Orientation orient):
    Glib::ObjectBase("KiranMenuListItem"),
    box(orient),
    space_property(*this, "icon-spacing", 0),
    max_width_property(*this, "max-width", G_MAXINT32),
    max_height_property(*this, "max-height", G_MAXINT32)
{
    init_ui();
}

MenuListItemWidget::MenuListItemWidget(const Glib::RefPtr<Gio::Icon> &gicon,
                                     const std::string &text,
                                     Gtk::Orientation orient,
                                     int icon_size):
    Glib::ObjectBase("KiranMenuListItem"),
    box(orient),
    space_property(*this, "icon-spacing", 0),
    max_width_property(*this, "max-width", G_MAXINT32),
    max_height_property(*this, "max-height", G_MAXINT32)
{
    init_ui();
    set_text(text);
    set_icon(gicon, icon_size);
}

void MenuListItemWidget::on_style_updated()
{
    box.set_spacing(space_property.get_value());
    Gtk::Button::on_style_updated();
}


bool MenuListItemWidget::on_draw(const::Cairo::RefPtr<Cairo::Context> &cr)
{
    auto context = get_style_context();
    Gtk::Allocation allocation;
    Gtk::Border margin;

    allocation = get_allocation();
    margin = context->get_margin(get_state_flags());

    Gtk::Button::on_draw(cr);
    context->render_focus(cr,
                          margin.get_left(),
                          margin.get_top(),
                          allocation.get_width() - margin.get_left() - margin.get_right(),
                          allocation.get_height() - margin.get_top() - margin.get_bottom());
    return false;
}

void MenuListItemWidget::on_orient_changed()
{
    if (box.get_orientation() == Gtk::ORIENTATION_HORIZONTAL)
        set_hexpand(true);
    else
        set_hexpand(false);
}

void MenuListItemWidget::get_preferred_width_vfunc(int &minimum_width,int &natural_width) const
{
    Gtk::Border padding, margin;
    int child_minimal_width, child_natural_width;
    auto context = get_style_context();

    padding = context->get_padding();
    margin = context->get_margin();

    do {
        //使用C接口读取CSS中的min-width和min-height设置
        GValue value = G_VALUE_INIT;
        GtkStyleContext *c_context = context.get()->gobj();

        gtk_style_context_get_property(c_context, "min-width", gtk_widget_get_state_flags(GTK_WIDGET(this->gobj())), &value);
        minimum_width = g_value_get_int(&value);
        g_value_unset(&value);
    } while(0);

    box.get_preferred_width(child_minimal_width, child_natural_width);
    if (!minimum_width)
        //如果CSS文件中没有指定最小大小，采用子控件大小
        minimum_width = child_minimal_width + padding.get_left() + padding.get_right() + margin.get_left() + margin.get_right();
    else
        minimum_width += margin.get_left() + margin.get_right();
    natural_width = child_natural_width + padding.get_left() + padding.get_right() + margin.get_left() + margin.get_right();

    if (minimum_width > max_width_property.get_value())
        minimum_width = max_width_property.get_value();

    if (natural_width < minimum_width)      //默认尺寸不能小于最小尺寸
        natural_width = minimum_width;

    if (natural_width > max_width_property.get_value())
        natural_width = max_width_property.get_value();
}
void MenuListItemWidget::get_preferred_height_vfunc(int &minimum_height,int &natural_height) const
{
    Gtk::Border padding, margin;
    int child_minimal_height, child_natural_height;
    auto context = get_style_context();

    padding = context->get_padding();
    margin = context->get_margin();

    do {
        GValue value = G_VALUE_INIT;
        GtkStyleContext *c_context = context.get()->gobj();

        gtk_style_context_get_property(c_context, "min-height", gtk_widget_get_state_flags(GTK_WIDGET(this->gobj())), &value);
        minimum_height = g_value_get_int(&value);
        g_value_unset(&value);
    } while(0);

    box.get_preferred_height(child_minimal_height, child_natural_height);

    if (minimum_height)
        minimum_height += margin.get_top() + margin.get_bottom();
    else
        minimum_height = child_minimal_height + padding.get_top() + padding.get_bottom() + margin.get_top() + margin.get_bottom();

    //从子控件的默认大小计算当前控件的默认大小
    natural_height = child_natural_height + padding.get_top() + padding.get_left() + margin.get_top() + margin.get_bottom();

    if (minimum_height >= max_height_property.get_value())
        minimum_height = max_height_property.get_value();

    if (natural_height < minimum_height)
        natural_height = minimum_height;

    if (natural_height > max_height_property.get_value())
        natural_height = max_height_property.get_value();
}

void MenuListItemWidget::set_text(const std::string &text)
{
    label.set_text(text);
}

void MenuListItemWidget::set_icon(const Glib::RefPtr<Gio::Icon> &gicon, int icon_size)
{
    image.set_pixel_size(icon_size);
    image.set_size_request(icon_size, icon_size);

    if (gicon)
        image.set(Glib::RefPtr<const Gio::Icon>::cast_dynamic(gicon), Gtk::ICON_SIZE_LARGE_TOOLBAR);
    else
        image.set_from_icon_name("gtk-missing", Gtk::ICON_SIZE_LARGE_TOOLBAR);
}

void MenuListItemWidget::set_icon(const std::string &resource, int icon_size)
{
    image.set_pixel_size(icon_size);
    image.set_from_resource(resource);
}

void MenuListItemWidget::set_icon_size(int icon_size)
{
    image.set_pixel_size(icon_size);
}

void MenuListItemWidget::set_orientation(Gtk::Orientation orient)
{
    if (orient == Gtk::ORIENTATION_HORIZONTAL)
        label.set_halign(Gtk::ALIGN_START);
    else
        label.set_halign(Gtk::ALIGN_CENTER);
    box.set_orientation(orient);
    box.queue_resize();
}

void MenuListItemWidget::init_ui()
{
    auto context = get_style_context();

    context->add_class("kiran-menu-list-item");

    label.set_halign(Gtk::ALIGN_START);
    label.set_ellipsize(Pango::ELLIPSIZE_END);
    box.pack_start(image, false, false);
    box.pack_start(label, true, true);
    box.property_orientation().signal_changed().connect(sigc::mem_fun(*this, &MenuListItemWidget::on_orient_changed));

    add(box);

    add_events(Gdk::ENTER_NOTIFY_MASK | Gdk::LEAVE_NOTIFY_MASK);
    set_can_focus(true);
}
