// This file is part of GtkEveMon.
//
// GtkEveMon is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// You should have received a copy of the GNU General Public License
// along with GtkEveMon. If not, see <http://www.gnu.org/licenses/>.

#ifndef GTK_SKILL_QUEUE_HEADER
#define GTK_SKILL_QUEUE_HEADER

#include <string>

#include <gtkmm.h>

#include "bits/character.h"
#include "gtkcolumnsbase.h"

class GtkSkillQueueColumns : public Gtk::TreeModel::ColumnRecord
{
  public:
    Gtk::TreeModelColumn<int> queue_pos;
    Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > skill_icon;
    Gtk::TreeModelColumn<Glib::ustring> skill_name;
    Gtk::TreeModelColumn<int> start_sp;
    Gtk::TreeModelColumn<int> end_sp;
    Gtk::TreeModelColumn<Glib::ustring> start_time;
    Gtk::TreeModelColumn<Glib::ustring> end_time;
    Gtk::TreeModelColumn<Glib::ustring> duration;
    Gtk::TreeModelColumn<Glib::ustring> training;
    Gtk::TreeModelColumn<Glib::ustring> attribs;
    Gtk::TreeModelColumn<int> skill_id;

    GtkSkillQueueColumns (void);
};

/* ---------------------------------------------------------------- */

class GtkSkillQueueViewCols : public GtkColumnsBase
{
  public:
    Gtk::TreeView::Column position;
    Gtk::TreeView::Column skill_name;
    Gtk::TreeView::Column start_sp;
    Gtk::TreeView::Column end_sp;
    Gtk::TreeView::Column start_time;
    Gtk::TreeView::Column end_time;
    Gtk::TreeView::Column duration;
    Gtk::TreeView::Column training;
    Gtk::TreeView::Column attribs;
    Gtk::TreeView::Column skill_id;

    GtkSkillQueueViewCols (Gtk::TreeView* view, GtkSkillQueueColumns* cols);
};

/* ---------------------------------------------------------------- */

class GtkSkillQueue : public Gtk::Box
{
  private:
    CharacterPtr character;

    GtkSkillQueueColumns queue_cols;
    Glib::RefPtr<Gtk::ListStore> queue_store;
    Gtk::TreeView queue_view;
    GtkSkillQueueViewCols queue_view_cols;

  protected:
    void on_apidata_available (void);
    void on_api_problems (EveApiDocType sheet, std::string error, bool cache);
    void raise_error (std::string const& error, bool cached);
    void init_from_config (void);
    void store_to_config (void);
    void on_row_activated (Gtk::TreeModel::Path const& path,
        Gtk::TreeViewColumn* col);

  public:
    GtkSkillQueue (void);
    ~GtkSkillQueue (void);

    void set_character (CharacterPtr character);
    void refresh (void);
};

/* ---------------------------------------------------------------- */

inline
GtkSkillQueueColumns::GtkSkillQueueColumns (void)
{
  this->add(this->queue_pos);
  this->add(this->skill_icon);
  this->add(this->skill_name);
  this->add(this->start_sp);
  this->add(this->end_sp);
  this->add(this->start_time);
  this->add(this->end_time);
  this->add(this->duration);
  this->add(this->training);
  this->add(this->attribs);
  this->add(this->skill_id);
}

#endif /* GTK_SKILL_QUEUE_HEADER */
