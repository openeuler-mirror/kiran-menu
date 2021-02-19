#include "recent-files-list-box.h"
#include "kiran-opacity-menu.h"
#include <glibmm/i18n.h>


RecentFilesListBox::RecentFilesListBox():
   filter_pattern("*")
{
   set_activate_on_single_click(false);
   set_filter_func(sigc::mem_fun(*this, &RecentFilesListBox::on_filter));

   get_style_context()->add_class("menu-recent-list");

   /* 最近访问文件列表发生变化时重新加载 */
   Gtk::RecentManager::get_default()->signal_changed().connect(
      sigc::mem_fun(*this, &RecentFilesListBox::load));

   load();
}

void RecentFilesListBox::set_filter_pattern(const Glib::ustring &filter_pattern_)
{
   g_debug("filter pattern is '%s'", filter_pattern_.c_str());
   filter_pattern = filter_pattern_;
   invalidate_filter();
}

void RecentFilesListBox::load()
{
   /* 清空原有信息 */
   while (true) {
      auto row = get_row_at_index(0);
      if (!row)
         break;
      
      remove(*row);
      delete row;
   }

   for (auto info: Gtk::RecentManager::get_default()->get_items()) {
      Gtk::ListBoxRow *row = nullptr;
      auto cell = create_recent_item(info);
      if (!cell)
         continue;

      row = Gtk::manage(new Gtk::ListBoxRow());
      row->add(*cell);
      row->set_data("info", info.get());
      row->show_all();
      this->insert(*row, -1);
   }
   invalidate_filter();
}

void RecentFilesListBox::on_row_activated(Gtk::ListBoxRow *row)
{
   Gtk::RecentInfo *info = (Gtk::RecentInfo*)row->get_data("info");

   if (info == nullptr) {
      g_warning("%s: no recent info found, ignore launch request", __func__);
      return;
   }
   open_file(Glib::RefPtr<Gtk::RecentInfo>(info));
}

void RecentFilesListBox::open_file(const Glib::RefPtr<Gtk::RecentInfo>& item)
{
   g_return_if_fail(item && !item->get_uri().empty());

   /* 使用系统默认方式打开文件 */
   Gio::AppInfo::launch_default_for_uri_async(item->get_uri());
}

void RecentFilesListBox::open_file_location(const Glib::RefPtr<Gtk::RecentInfo>& item) 
{
   g_return_if_fail(item && !item->get_uri().empty());

   auto file = Gio::File::create_for_uri(item->get_uri());
   auto dir = file->get_parent();

   if (dir)
      Gio::AppInfo::launch_default_for_uri_async(dir->get_uri());
   else
      g_warning("%s: no parent found for file '%s'", __func__, item->get_uri());
}

void RecentFilesListBox::remove_file_from_list(const Glib::RefPtr<Gtk::RecentInfo>& item) 
{
   g_return_if_fail(item && !item->get_uri().empty());
   auto manager = Gtk::RecentManager::get_default();
   manager->remove_item(item->get_uri());
}

void RecentFilesListBox::clear_files_list(void) 
{
   auto manager = Gtk::RecentManager::get_default();

   Gtk::MessageDialog dialog(
         _("All recent files information will be lost.\nAre you sure to clear the whole lists?"),
         true,
         Gtk::MESSAGE_QUESTION,
         Gtk::BUTTONS_YES_NO,
         true);

   dialog.set_title(_("System Warning"));
   if (dialog.run() == Gtk::RESPONSE_YES)
      manager->purge_items();
}

/**
 * @brief 根据最近的文档信息item创建对应的列表项，列表中将显示文档图标和文档名称 
 * @param item 要创建列表项的文档
 * @return  成功返回创建的列表项，失败返回nullptr
 */
Gtk::Widget* RecentFilesListBox::create_recent_item(const Glib::RefPtr<Gtk::RecentInfo>& item) 
{
   
   Gtk::EventBox *widget;
   Gtk::Box *box;
   Gtk::Image *image;
   Gtk::Label *label;

   g_return_val_if_fail(item, nullptr);

   image = Gtk::manage(new Gtk::Image(item->get_icon(24)));
   label = Gtk::manage(new Gtk::Label(item->get_display_name()));
   box = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 10));
   widget = Gtk::manage(new Gtk::EventBox());

   auto context_menu = create_context_menu(item);

   g_assert(context_menu != nullptr);
   context_menu->attach_to_widget(*widget);

   label->set_ellipsize(Pango::ELLIPSIZE_END);
   label->set_xalign(0.0f);

   widget->add_events(Gdk::KEY_PRESS_MASK);
   widget->set_tooltip_text(item->get_display_name());
   widget->get_style_context()->add_class("row-box");
   widget->signal_button_press_event().connect_notify(
      [widget, context_menu, this](const GdkEventButton *button_event)-> void {
         const GdkEvent* event = (const GdkEvent*)button_event;
         if (gdk_event_triggers_context_menu(event)) {
            /* 右键点击时，选择点击项并显示右键菜单 */
            Gtk::ListBoxRow *row_ = (Gtk::ListBoxRow*)widget->get_parent();
            this->select_row(*row_);
            context_menu->popup_at_pointer(event);
         }
      });

   
   box->add(*image);
   box->add(*label);
   widget->add(*box);

   return widget;
}

/**
 * @brief 为给定的最近访问文档创建右键菜单
 * @param info 需要创建右键菜单的文档
 * @return  返回创建的菜单控件, 如果info为空，返回nullptr
 */
Gtk::Menu* RecentFilesListBox::create_context_menu(const Glib::RefPtr<Gtk::RecentInfo> &info) 
{
   Gtk::MenuItem *item = nullptr;

   g_return_val_if_fail(info, nullptr);

   auto menu = new KiranOpacityMenu();

   item = Gtk::manage(new Gtk::MenuItem(_("Open File")));
   item->signal_activate().connect(
       sigc::bind(sigc::ptr_fun(&RecentFilesListBox::open_file), info));
   menu->append(*item);

   item = Gtk::manage(new Gtk::MenuItem(_("Open File Location")));
   item->signal_activate().connect(
       sigc::bind(sigc::ptr_fun(&RecentFilesListBox::open_file_location), info));
   menu->append(*item);

   item = Gtk::manage(new Gtk::MenuItem(_("Remove from list")));
   item->signal_activate().connect(
       sigc::bind(sigc::ptr_fun(&RecentFilesListBox::remove_file_from_list), info));
   menu->append(*item);

   item = Gtk::manage(new Gtk::MenuItem(_("Clear the whole list")));
   item->signal_activate().connect(
       sigc::ptr_fun(&RecentFilesListBox::clear_files_list));
   menu->append(*item);

   menu->show_all();
   return menu;
}

bool RecentFilesListBox::on_filter(Gtk::ListBoxRow* row) 
{
   GtkRecentFilterInfo filter_info;
   Gtk::RecentInfo *info = (Gtk::RecentInfo*)row->get_data("info");

   if (info == nullptr || filter_pattern == "*")
      return true;

   auto filter = Gtk::RecentFilter::create();

   filter->add_pattern(filter_pattern);
   filter_info.contains = gtk_recent_filter_get_needed(filter->gobj());
   filter_info.display_name = info->get_display_name().c_str();

   return gtk_recent_filter_filter(filter->gobj(), &filter_info);
}
