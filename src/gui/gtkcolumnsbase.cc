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

#include <gtkmm.h>

#include "util/exception.h"
#include "util/helpers.h"

#include "gtkcolumnsbase.h"

void
GtkColumnsFormat::set_format (std::string const& format_str)
{
  this->clear();
  std::string fstr = format_str;

  while (true)
  {
    if (fstr.empty())
      break;

    /* Find blank and separate parts. */
    std::string fpart;
    size_t off = fstr.find_first_of(' ');
    if (off == std::string::npos)
    {
      fpart = fstr;
      fstr.clear();
    }
    else
    {
      fpart = fstr.substr(0, off);
      fstr = fstr.substr(off + 1);
    }

    /* Check if the column is visible. */
    bool visible;
    if (fpart[0] == '+')
      visible = true;
    else if (fpart[0] == '-')
      visible = false;
    else
    {
      std::cout << "GtkColumnsBase: Format parse error 1" << std::endl;
      throw Exception("GtkColumnsBase: Format parse error 1");
    }

    fpart = fpart.substr(1);

    int column;
    try
    {
      column = Helpers::get_int_from_string(fpart);
    }
    catch (Exception& e)
    {
      std::cout << "GtkColumnsBase: Format parse error 2" << std::endl;
      throw Exception("GtkColumnsBase: Format parse error 2");
    }

    this->push_back(std::make_pair(column, visible));
  }

  //if (!this->check_format())
  //  throw Exception("GtkColumnsBase: Invalid format string!");
}

/* ---------------------------------------------------------------- */

std::string
GtkColumnsFormat::get_format (void) const
{
  std::string ret;
  ret.reserve(3 * this->size());
  for (unsigned int i = 0; i < this->size(); ++i)
  {
    ret += (this->at(i).second ? "+" : "-");
    ret += Helpers::get_string_from_int((int)this->at(i).first);
    if (i < this->size() - 1)
      ret += " ";
  }

  return ret;
}

/* ---------------------------------------------------------------- */
/* The following algo will check for dupes, valid values and holes
 * in the column format vector. In other words: The code will return
 * true for any valid permutation of {0, ..., this->size() - 1} for
 * the column positions.
 */

bool
GtkColumnsFormat::check_format (void)
{
  std::vector<bool> col_exist;
  col_exist.resize(this->size(), false);

  for (unsigned int i = 0; i < this->size(); ++i)
  {
    int col_pos = this->at(i).first;
    if ((unsigned int)col_pos >= this->size() || col_pos < 0)
      return false;
    col_exist[col_pos] = true;
  }

  for (unsigned int i = 0; i < this->size(); ++i)
    if (col_exist[i] == false)
      return false;

  return true;
}

/* ================================================================ */

GtkColumnsBase::GtkColumnsBase (Gtk::TreeView* view)
{
  this->tree_view = view;
  this->edit_context = false;
}

/* ---------------------------------------------------------------- */

void
GtkColumnsBase::append_column (Gtk::TreeViewColumn* col, GtkColumnOptions o)
{
  this->columns.push_back(std::make_pair(col, o));
  if (this->columns.size() > this->format.size())
    this->format.push_back(std::make_pair((int)this->format.size(), true));
}

/* ---------------------------------------------------------------- */

void
GtkColumnsBase::setup_columns_editable (void)
{
  if (this->format.size() != this->columns.size())
  {
    std::cout << "GtkColumnsBase: Internal size mismatch!" << std::endl;
    throw Exception("GtkColumnsBase: Internal size mismatch!");
  }
  /* If we previously was in normal context, store format positions. */
  if (!this->edit_context)
    this->update_format_positions();

  this->tree_view->remove_all_columns();

  for (unsigned int i = 0; i < this->format.size(); ++i)
  {
    GtkColumnsPair& colpair = this->columns[this->format[i].first];

    /* Only removable columns get new widgets. */
    if (colpair.second.removable)
    {
      /* Setup checkbutton widget. */
      Gtk::CheckButton* cb = Gtk::manage(new Gtk::CheckButton
            (colpair.first->get_title(), false));
      cb->set_active(this->format[i].second);
      colpair.first->set_widget(*cb);
      colpair.first->set_clickable(false);
      this->tree_view->append_column(*colpair.first);
      cb->show_all();
    }
    else if (colpair.second.icon)
    {
      Gtk::Widget* w = Gtk::manage(new Gtk::Image(colpair.second.icon));
      colpair.first->set_widget(*w);
      this->tree_view->append_column(*colpair.first);
      w->show_all();
    }
    else
    {
      Gtk::Label* lb = Gtk::manage
          (new Gtk::Label(colpair.first->get_title()));
      colpair.first->set_widget(*lb);
      this->tree_view->append_column(*colpair.first);
      lb->show_all();
    }

    colpair.first->set_reorderable(false);
  }

  this->edit_context = true;
}

/* ---------------------------------------------------------------- */

void
GtkColumnsBase::setup_columns_normal (void)
{
  if (this->format.size() != this->columns.size())
  {
    std::cout << "GtkColumnsBase: Internal size mismatch!" << std::endl;
    throw Exception("GtkColumnsBase: Internal size mismatch!");
  }

  /* If we previously was in edit context, store format visibility. */
  if (this->edit_context)
    this->update_format_visibility();

  this->tree_view->remove_all_columns();

  for (unsigned int i = 0; i < this->format.size(); ++i)
  {
    GtkColumnsPair& colpair = this->columns[this->format[i].first];

    if (!colpair.second.icon)
    {
      /* Setup label widget. */
      Gtk::Label* lb = Gtk::manage(new Gtk::Label(colpair.first->get_title()));
      colpair.first->set_widget(*lb);
      lb->show_all();
    }
    else
    {
      /* Setup image widget. */
      Gtk::Widget* w = Gtk::manage(new Gtk::Image(colpair.second.icon));
      colpair.first->set_widget(*w);
      w->show_all();
    }

    colpair.first->set_clickable(colpair.second.clickable);
    colpair.first->set_reorderable(colpair.second.reorderable);
  }

  this->show_format_columns();
  this->edit_context = false;
}

/* ---------------------------------------------------------------- */

void
GtkColumnsBase::toggle_edit_context (void)
{
  if (this->edit_context == true)
    this->setup_columns_normal();
  else
    this->setup_columns_editable();
}

/* ---------------------------------------------------------------- */

std::string
GtkColumnsBase::get_format (void)
{
  this->update_format_positions();
  if (this->edit_context)
    this->update_format_visibility();
  return this->format.get_format();
}

/* ---------------------------------------------------------------- */

void
GtkColumnsBase::set_format (std::string const& format_str)
{
  this->format.set_format(format_str);
  this->check_format();
}

/* ---------------------------------------------------------------- */

void
GtkColumnsBase::show_format_columns (void)
{
  for (unsigned int i = 0; i < this->format.size(); ++i)
    /* If the colum is visible. */
    if (this->format[i].second)
    {
      /* If it is editable append it editable. */
      this->tree_view->append_column
          (*this->columns[this->format[i].first].first);
    }
}

/* ---------------------------------------------------------------- */

void
GtkColumnsBase::check_format (void)
{
  while (this->columns.size() > this->format.size())
    this->format.push_back(std::make_pair((int)this->format.size(), true));

  while (this->columns.size() < this->format.size())
    this->format.pop_back();

  if (!this->format.check_format())
  {
    std::cout << "GtkColumnsBase: Invalid format!" << std::endl;
    throw Exception("GtkColumnsBase: Invalid format!");
  }
}

/* ---------------------------------------------------------------- */

void
GtkColumnsBase::update_format_positions (void)
{
  Glib::ListHandle<Gtk::TreeViewColumn*> clist = this->tree_view->get_columns();
  if (clist.size() == 0)
    return;

  /* Determine current column order. */
  std::vector<int> view_order;
  Glib::ListHandle<Gtk::TreeViewColumn*>::iterator iter = clist.begin();
  while (iter != clist.end())
  {
    for (unsigned int i = 0; i < this->columns.size(); ++i)
      if (this->columns[i].first == *iter)
      {
        view_order.push_back(i);
        break;
      }
    iter++;
  }

  if (clist.size() != view_order.size())
  {
    std::cout << "GtkColumnsBase: Columns size mismatch. "
        "Cannot determine order" << std::endl;
    return;
  }

  /* Update the format with the current order. */
  unsigned int current_col = 0;
  for (unsigned int i = 0; i < this->format.size(); ++i)
    for (unsigned int j = 0; j < view_order.size(); ++j)
      if (this->format[i].first == view_order[j])
      {
        this->format[i].first = view_order[current_col];
        current_col += 1;
        if (current_col == view_order.size())
          return;
        else
          break;
      }
}

/* ---------------------------------------------------------------- */

void
GtkColumnsBase::update_format_visibility (void)
{
  Glib::ListHandle<Gtk::TreeViewColumn*> clist = this->tree_view->get_columns();
  if (clist.size() != this->format.size())
  {
    std::cout << "ListViewColumns: Invalid column state" << std::endl;
    return;
  }

  for (unsigned int i = 0; i < this->format.size(); ++i)
    if (this->columns[this->format[i].first].second.removable)
      this->format[i].second = ((Gtk::CheckButton*)this->columns
          [this->format[i].first].first->get_widget())->get_active();
}
