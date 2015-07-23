// This file is part of GtkEveMon.
//
// GtkEveMon is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// You should have received a copy of the GNU General Public License
// along with GtkEveMon. If not, see <http://www.gnu.org/licenses/>.

#ifndef GTK_CONF_WIDGETS_HEADER
#define GTK_CONF_WIDGETS_HEADER

#include <string>

#include <gtkmm.h>

#include "bits/config.h"

class GtkConfFileChooser : public Gtk::FileChooserButton
{
  private:
    ConfValuePtr value;

    void on_selection_change (void);

  public:
    GtkConfFileChooser (std::string const& key, Gtk::FileChooserAction action);
};

/* ---------------------------------------------------------------- */

class GtkConfCheckButton : public Gtk::CheckButton
{
  private:
    ConfValuePtr value;

    void on_button_toggled (void);

  public:
    GtkConfCheckButton (const Glib::ustring& label, bool mnemonic,
        std::string const& conf_key);
};

/* ---------------------------------------------------------------- */

class GtkConfTextEntry : public Gtk::Entry
{
  private:
    ConfValuePtr value;

    void on_text_changed (void);

  public:
    GtkConfTextEntry (std::string const& conf_key);
};

/* ---------------------------------------------------------------- */

class GtkConfComboBox : public Gtk::ComboBoxText
{
  private:
    ConfValuePtr value;
    std::vector<std::string> values;

    void on_changed_signal (void);

  public:
    GtkConfComboBox (std::string const& conf_key);
    void append_conf_row (std::string const& text, std::string const& value);
};

/* ---------------------------------------------------------------- */

class GtkConfSectionSelection : public Gtk::ComboBox
{
  private:
    struct GtkConfSelectionCols : public Gtk::TreeModelColumnRecord
    {
      Gtk::TreeModelColumn<Glib::ustring> name;
      Gtk::TreeModelColumn<ConfSectionPtr> section;
      GtkConfSelectionCols (void)
      { this->add(name); this->add(section); }
    } selection_cols;

  private:
    ConfSectionPtr parent_section;

    sigc::connection changed_conn;
    Glib::RefPtr<Gtk::ListStore> selection_store;
    sigc::signal<void, ConfSectionPtr> sig_conf_section_changed;

  protected:
    void on_combo_entry_changed (void);
    void update_selection_store (std::string const& select = "");

  public:
    GtkConfSectionSelection (void);
    void set_parent_config_section (std::string const& section,
        std::string const& select = "");

    ConfSectionPtr create_new_section (std::string const& name);
    ConfSectionPtr get_active_section (void);
    Glib::ustring get_active_name (void);
    void set_active_section (std::string const& name);
    void delete_section (std::string const& name);
    void rename_section (std::string const& from, std::string const& to);

    sigc::signal<void, ConfSectionPtr>& signal_conf_section_changed (void);
};

#endif /* GTK_CONF_WIDGETS_HEADER */
