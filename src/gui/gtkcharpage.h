// This file is part of GtkEveMon.
//
// GtkEveMon is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// You should have received a copy of the GNU General Public License
// along with GtkEveMon. If not, see <http://www.gnu.org/licenses/>.

#ifndef GTK_CHAR_PAGE_HEADER
#define GTK_CHAR_PAGE_HEADER

#include <string>

#include <gdkmm.h>
#include <gtkmm.h>

#include "bits/character.h"

#include "gtkportrait.h"
#include "gtkinfodisplay.h"

/* Update the live SP labels every this milli seconds. */
#define CHARPAGE_LIVE_SP_LABEL_UPDATE 1000
/* Update the live SP image every this milli seconds. */
#define CHARPAGE_LIVE_SP_IMAGE_UPDATE 60000
/* Check for expired sheets every this milli seconds. */
#define CHARPAGE_CHECK_EXPIRED_SHEETS 600000
/* Update the cached duration this milli seconds. */
#define CHARPAGE_UPDATE_CACHED_DURATION 25000

class GtkCharSkillsCols : public Gtk::TreeModel::ColumnRecord
{
  public:
    GtkCharSkillsCols (void);

    Gtk::TreeModelColumn<int> id;
    Gtk::TreeModelColumn<ApiCharSheetSkill*> skill;
    Gtk::TreeModelColumn<Glib::ustring> name;
    Gtk::TreeModelColumn<Glib::ustring> points;
    Gtk::TreeModelColumn<Glib::ustring> max_points;
    Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > level;
    Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > icon;
    Gtk::TreeModelColumn<Glib::ustring> primary;
    Gtk::TreeModelColumn<Glib::ustring> secondary;
};

/* ---------------------------------------------------------------- */

class GtkCharPage : public Gtk::Box
{
  private:
    /* Character to be monitored. */
    CharacterPtr character;

    /* GUI stuff. */
    Gtk::Window* parent_window;
    Gtk::Label char_name_label;
    Gtk::Label char_info_label;
    Gtk::Label corp_label;
    Gtk::Label balance_label;
    Gtk::Label skill_points_label;
    Gtk::Label known_skills_label;
    Gtk::Label attr_cha_label;
    Gtk::Label attr_int_label;
    Gtk::Label attr_per_label;
    Gtk::Label attr_mem_label;
    Gtk::Label attr_wil_label;
    Gtk::Label training_label;
    Gtk::Label remaining_label;
    Gtk::Label finish_eve_label;
    Gtk::Label finish_local_label;
    Gtk::Label spph_label;
    Gtk::Label live_sp_label;
    Gtk::Label charsheet_info_label;
    Gtk::Label skillqueue_info_label;
    Gtk::Button refresh_but;
    Gtk::Button info_but;
    GtkPortrait char_image;
    GtkInfoDisplay info_display;

    GtkCharSkillsCols skill_cols;
    Glib::RefPtr<Gtk::TreeStore> skill_store;
    Gtk::TreeView skill_view;
    Glib::RefPtr<Gtk::StatusIcon> tray_notify;

    /* Cached tree iterators for fast skill and group update. */
    Gtk::TreeIter tree_skill_iter;
    Gtk::TreeIter tree_group_iter;

    /* Helpers, signal handlers, etc. */
    void update_charsheet_details (void);
    void update_training_details (void);
    void update_skill_list (void);
    void delete_skill_completed_dialog (int response, Gtk::Widget* widget);

    /* Request and process EVE API documents. */
    void request_documents (void);
    bool check_expired_sheets (void);

    /* Error dialogs. */
    void on_skilltree_error (std::string const& e);
    void on_api_error (EveApiDocType dt, std::string msg, bool cached);
    void popup_error_dialog (std::string const& title,
        std::string const& heading, std::string const& message);

    /* Misc GUI stuff. */
    bool update_remaining (void);
    bool update_cached_duration (void);
    void api_info_changed (void);
    void remove_tray_notify (void);
    void create_tray_notify (void);
    void exec_notification_handler (void);
    void on_skill_completed (void);
    void on_close_clicked (void);
    void on_info_clicked (void);
    void on_skillqueue_clicked (void);
    bool on_query_skillview_tooltip (int x, int y, bool key,
        Glib::RefPtr<Gtk::Tooltip> const& tooltip);
    void on_skill_activated (Gtk::TreeModel::Path const& path,
        Gtk::TreeViewColumn* col);

    bool on_live_sp_value_update (void);
    bool on_live_sp_image_update (void);

  public:
    GtkCharPage (CharacterPtr character);

    CharacterPtr get_character (void) const;
    void set_parent_window (Gtk::Window* parent);
};

/* ---------------------------------------------------------------- */

inline
GtkCharSkillsCols::GtkCharSkillsCols (void)
{
  this->add(this->id);
  this->add(this->skill);
  this->add(this->name);
  this->add(this->points);
  this->add(this->max_points);
  this->add(this->level);
  this->add(this->icon);
  this->add(this->primary);
  this->add(this->secondary);
}

inline void
GtkCharPage::set_parent_window (Gtk::Window* parent)
{
  this->parent_window = parent;
}

inline CharacterPtr
GtkCharPage::get_character (void) const
{
  return this->character;
}

inline void
GtkCharPage::delete_skill_completed_dialog (int, Gtk::Widget* widget)
{
  delete widget;
  this->remove_tray_notify();
}

#endif /* GTK_CHAR_PAGE_HEADER */
