// This file is part of GtkEveMon.
//
// GtkEveMon is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// You should have received a copy of the GNU General Public License
// along with GtkEveMon. If not, see <http://www.gnu.org/licenses/>.

#ifndef GUI_USER_DATA_HEADER
#define GUI_USER_DATA_HEADER

#include <string>
#include <gtkmm.h>

#include "api/eveapi.h"
#include "winbase.h"

class GuiUserDataColRecord : public Gtk::TreeModel::ColumnRecord
{
  public:
    GuiUserDataColRecord (void);

    Gtk::TreeModelColumn<bool> selected;
    Gtk::TreeModelColumn<Glib::ustring> name;
    Gtk::TreeModelColumn<Glib::ustring> charid;
    Gtk::TreeModelColumn<Glib::ustring> corp;
};

/* ---------------------------------------------------------------- */

class GuiUserDataComboColRecord : public Gtk::TreeModel::ColumnRecord
{
  public:
    GuiUserDataComboColRecord (void);

    Gtk::TreeModelColumn<Glib::ustring> userid;
    Gtk::TreeModelColumn<Glib::ustring> chars;
};

/* ---------------------------------------------------------------- */

class GuiUserData : public WinBase
{
  private:
    EveApiFetcher charlist_fetcher;

    Gtk::Entry userid_entry;
    Gtk::Entry apikey_entry;
    Gtk::Button apply_button;
    Gtk::CheckButton api_v1_cb;

    GuiUserDataComboColRecord combo_cols;
    Glib::RefPtr<Gtk::ListStore> combo_store;
    Gtk::ComboBox history_box;

    GuiUserDataColRecord char_cols;
    Glib::RefPtr<Gtk::ListStore> char_store;
    Gtk::TreeView char_list;

    void init_from_config (void);
    void on_add_clicked (void);
    void on_apply_clicked (void);
    void on_history_selected (void);
    void on_charlist_available (EveApiData data);
    void print_error (std::string const& error, bool cached = false);

  public:
    GuiUserData (void);
};

/* ---------------------------------------------------------------- */

inline
GuiUserDataColRecord::GuiUserDataColRecord (void)
{
  this->add(this->selected);
  this->add(this->name);
  this->add(this->charid);
  this->add(this->corp);
}

inline
GuiUserDataComboColRecord::GuiUserDataComboColRecord (void)
{
  this->add(this->userid);
  this->add(this->chars);
}

#endif /* GUI_USER_DATA_HEADER */
