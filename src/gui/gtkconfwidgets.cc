// This file is part of GtkEveMon.
//
// GtkEveMon is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// You should have received a copy of the GNU General Public License
// along with GtkEveMon. If not, see <http://www.gnu.org/licenses/>.

#include <iostream>

#include "bits/config.h"
#include "gtkportrait.h"
#include "gtkconfwidgets.h"

GtkConfFileChooser::GtkConfFileChooser (std::string const& key,
    Gtk::FileChooserAction action)
  : Gtk::FileChooserButton(action)
{
  this->value = Config::conf.get_value(key);

  if ((**this->value).empty())
    this->set_filename("/");
  else
    this->set_filename(**this->value);

  this->signal_selection_changed().connect(sigc::mem_fun
      (*this, &GtkConfFileChooser::on_selection_change));
}

/* ---------------------------------------------------------------- */

void
GtkConfFileChooser::on_selection_change (void)
{
  this->value->set(this->get_filename());
}

/* ================================================================ */

GtkConfCheckButton::GtkConfCheckButton (Glib::ustring const& label,
    bool mnemonic, std::string const& conf_key)
  : Gtk::CheckButton(label, mnemonic)
{
  this->value = Config::conf.get_value(conf_key);
  this->set_active(this->value->get_bool());

  this->signal_toggled().connect(sigc::mem_fun
      (*this, &GtkConfCheckButton::on_button_toggled));
}

/* ---------------------------------------------------------------- */

void
GtkConfCheckButton::on_button_toggled (void)
{
  this->value->set(this->get_active());
}

/* ================================================================ */

GtkConfTextEntry::GtkConfTextEntry (std::string const& conf_key)
{
  this->value = Config::conf.get_value(conf_key);
  this->set_text(value->get_string());

  this->signal_changed().connect(sigc::mem_fun
      (*this, &GtkConfTextEntry::on_text_changed));
}

/* ---------------------------------------------------------------- */

void
GtkConfTextEntry::on_text_changed (void)
{
  this->value->set(this->get_text());
}

/* ================================================================ */

GtkConfComboBox::GtkConfComboBox (std::string const& conf_key)
{
  this->value = Config::conf.get_value(conf_key);
  this->signal_changed().connect(sigc::mem_fun(*this,
      &GtkConfComboBox::on_changed_signal));
}

/* ---------------------------------------------------------------- */

void
GtkConfComboBox::on_changed_signal (void)
{
  this->value->set(this->values[this->get_active_row_number()]);
}

/* ---------------------------------------------------------------- */

void
GtkConfComboBox::append_conf_row (std::string const& text,
    std::string const& value)
{
  this->values.push_back(value);
  this->append(text);

  if (value == this->value->get_string())
    this->set_active((int)this->values.size() - 1);
}

/* ================================================================ */

GtkConfSectionSelection::GtkConfSectionSelection (void)
  : selection_store(Gtk::ListStore::create(selection_cols))
{
  this->set_model(this->selection_store);
  this->pack_start(this->selection_cols.name, true);

  this->changed_conn = this->signal_changed().connect(sigc::mem_fun
      (*this, &GtkConfSectionSelection::on_combo_entry_changed));
}

/* ---------------------------------------------------------------- */

void
GtkConfSectionSelection::set_parent_config_section (std::string const& section,
    std::string const& select)
{
  this->parent_section = Config::conf.get_or_create_section(section);
  this->update_selection_store(select);
}

/* ---------------------------------------------------------------- */

void
GtkConfSectionSelection::update_selection_store (std::string const& select)
{
  /* Stop signalling. This one emits a lot of on_combo_entry_changed */
  this->changed_conn.block();
  this->selection_store->clear();
  this->changed_conn.unblock();

  bool selected = false;
  conf_sections_t::iterator iter;
  for (iter = this->parent_section->sections_begin();
      iter != this->parent_section->sections_end(); iter++)
  {
    Gtk::ListStore::iterator row = this->selection_store->append();
    (*row)[this->selection_cols.name] = iter->first;
    (*row)[this->selection_cols.section] = iter->second;
    if (!select.empty() && iter->first == select)
    {
      this->set_active(row);
      selected = true;
    }
  }

  if (!selected)
  {
    if (this->selection_store->children().size() > 0)
      this->set_active(0);
    else
    {
      this->set_active(-1);
      this->on_combo_entry_changed();
    }
  }
}

/* ---------------------------------------------------------------- */

void
GtkConfSectionSelection::on_combo_entry_changed (void)
{
  if (this->get_active_row_number() == -1)
  {
    this->sig_conf_section_changed.emit(ConfSectionPtr());
  }
  else
  {
    ConfSectionPtr section = this->get_active_section();
    this->sig_conf_section_changed.emit(section);
  }
}

/* ---------------------------------------------------------------- */

ConfSectionPtr
GtkConfSectionSelection::create_new_section (std::string const& name)
{
  ConfSectionPtr section;
  try
  {
    /* Check if section exists. Select this if existing. */
    section = this->parent_section->get_section(name);
    this->update_selection_store(name);
  }
  catch (Exception& e)
  {
    /* Section does not exist. */
    section = ConfSection::create();
    this->parent_section->add(name, section);
    this->update_selection_store(name);
  }

  return section;
}

/* ---------------------------------------------------------------- */

ConfSectionPtr
GtkConfSectionSelection::get_active_section (void)
{
  if (this->get_active_row_number() == -1)
    return ConfSectionPtr();

  Gtk::ListStore::iterator iter = this->get_active();
  return (*iter)[this->selection_cols.section];
}

/* ---------------------------------------------------------------- */

Glib::ustring
GtkConfSectionSelection::get_active_name (void)
{
  if (this->get_active_row_number() == -1)
    return "";

  Gtk::ListStore::iterator iter = this->get_active();
  return (*iter)[this->selection_cols.name];
}

/* ---------------------------------------------------------------- */

void
GtkConfSectionSelection::set_active_section (std::string const& name)
{
  Gtk::ListStore::iterator row;
  for (row = this->selection_store->children().begin();
      row != this->selection_store->children().end(); row++)
  {
    if ((*row)[this->selection_cols.name] == name)
    {
      this->set_active(row);
      return;
    }
  }
}

/* ---------------------------------------------------------------- */

void
GtkConfSectionSelection::delete_section (std::string const& name)
{
  this->parent_section->remove_section(name);
  this->update_selection_store();
}

/* ---------------------------------------------------------------- */

void
GtkConfSectionSelection::rename_section (std::string const& from,
    std::string const& to)
{
  if (from.empty() || to.empty())
    throw Exception("Old name or new name not specified!");

  if (from == to)
    throw Exception("Old name and new name are identical!");

  ConfSectionPtr sect;
  try
  {
    sect = this->parent_section->get_section(from);
  }
  catch (Exception& e)
  {
    throw Exception("Old section name not found!");
  }

  try
  {
    /* Check if destination exists. */
    ConfSectionPtr dest = this->parent_section->get_section(to);
    throw Exception("New section name already exists!");
  }
  catch (Exception& e)
  {
  }

  this->parent_section->add(to, sect);
  this->parent_section->remove_section(from);
  this->update_selection_store(to);
}

/* ---------------------------------------------------------------- */

sigc::signal<void, ConfSectionPtr>&
GtkConfSectionSelection::signal_conf_section_changed (void)
{
  return this->sig_conf_section_changed;
}
