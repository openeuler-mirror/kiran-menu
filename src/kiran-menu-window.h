#ifndef KIRANMENUWINDOW_H
#define KIRANMENUWINDOW_H

#include <gtkmm.h>
#include "kiran-user-info.h"
#include "kiran-menu-app-item.h"
#include "kiran-menu-category-item.h"
#include "kiran-menu-profile.h"

#include "menu-skeleton.h"

class KiranMenuWindow : public Gtk::Window
{
public:
    KiranMenuWindow(Gtk::WindowType window_type = Gtk::WINDOW_TOPLEVEL);
    ~KiranMenuWindow();

    //尺寸变化信号
    sigc::signal<void,int,int> signal_size_changed();

    void reload_apps_data();
    void load_favorite_apps();
    void load_frequent_apps();
    void load_new_apps();

    //设置开始菜单显示模式(紧凑或扩展）
    void set_display_mode(MenuDisplayMode mode);

protected:
    virtual bool on_map_event(GdkEventAny *any_event) override;
    virtual bool on_unmap_event(GdkEventAny *any_event) override;
    virtual bool on_leave_notify_event(GdkEventCrossing *crossing_event) override;
    virtual bool on_key_press_event(GdkEventKey *key_event) override;
    virtual bool on_button_press_event(GdkEventButton *button_event) override;
    virtual bool on_configure_event(GdkEventConfigure* configure_event) override;
    virtual void on_realize();
    void on_active_change();

    //virtual bool on_draw(const Cairo::RefPtr<Cairo::Context> &cr) override;

    void on_search_change();
    void on_search_stop();

    void switch_to_category_overview(const std::string &selected_category);
    void switch_to_apps_overview(const std::string &selected_category, bool animation = true);
    void switch_to_apps_overview(double position, bool animation = true);
    void switch_to_compact_favorites_view(bool animation = true);

    bool promise_item_viewable(GdkEventFocus *event, Gtk::Widget *item);

    void check_size();

private:
    Glib::RefPtr<Gtk::Builder> builder;

    Gtk::Box *box;
    Gtk::SearchEntry *search_entry;
    Gtk::Grid *side_box;
    Gtk::Stack *overview_stack, *appview_stack;
    Gtk::Box *all_apps_box, *new_apps_box;
    Gtk::Box *favorite_apps_box, *frequent_apps_box;
    Gtk::Grid *category_overview_box, *search_results_box;
    Gtk::Box *compact_tab_box;

    Gdk::Rectangle geometry;

    KiranUserInfo *user_info;
    std::vector<std::string> category_names;
    std::map<std::string, KiranMenuCategoryItem*> category_items;

    Gtk::StyleProperty<int> compact_min_height_property, expand_min_height_property;

    KiranMenuProfile profile;
    Kiran::MenuSkeleton *backend;
    MenuDisplayMode display_mode;
    sigc::signal<void,int,int> m_signal_size_changed;

    void add_app_button(const char *icon_resource,
                        const char *tooltip,
                        const char *cmdline);

    void add_app_tab(const char *icon_resource,
                     const char *tooltip,
                     const char *page);
    void create_empty_prompt_label(Gtk::Label* &label, const char *prompt_text);

    void add_sidebar_buttons();

    void load_all_apps();
    void load_date_info();
    void load_user_info();

    KiranMenuAppItem *create_app_item(std::shared_ptr<Kiran::App> app,
                                  Gtk::Orientation orient = Gtk::ORIENTATION_HORIZONTAL);
    KiranMenuCategoryItem *create_category_item(const std::string &name, bool clickable=true);
};

#endif // KIRANMENUWINDOW_H
