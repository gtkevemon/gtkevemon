// This file is part of GtkEveMon.
//
// GtkEveMon is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// You should have received a copy of the GNU General Public License
// along with GtkEveMon. If not, see <http://www.gnu.org/licenses/>.

#ifndef GTK_COLUMNS_BASE_HEADER
#define GTK_COLUMNS_BASE_HEADER

#include <vector>
#include <string>
#include <utility>

#include <gdkmm.h>
#include <gtkmm.h>

class GtkColumnOptions
{
  public:
    bool removable;
    bool clickable;
    bool reorderable;
    Glib::RefPtr<Gdk::Pixbuf> icon;

    GtkColumnOptions (void);
    GtkColumnOptions (bool rem, bool click, bool reorder,
        Glib::RefPtr<Gdk::Pixbuf> icon = Glib::RefPtr<Gdk::Pixbuf>());
};

/* ---------------------------------------------------------------- */

typedef std::vector<std::pair<int, bool> > GtkColumnsFormatVector;
class GtkColumnsFormat : public GtkColumnsFormatVector
{
  public:
    void set_format (std::string const& format_str);
    std::string get_format (void) const;
    bool check_format (void);
};

/* ---------------------------------------------------------------- */

typedef std::pair<Gtk::TreeViewColumn*, GtkColumnOptions> GtkColumnsPair;
class GtkColumnsBase
{
  private:
    Gtk::TreeView* tree_view;
    std::vector<GtkColumnsPair> columns;
    GtkColumnsFormat format;
    bool edit_context;

    void show_format_columns (void);
    void check_format (void);
    void update_format_positions (void);
    void update_format_visibility (void);

  public:
    GtkColumnsBase (Gtk::TreeView* view);
    void append_column (Gtk::TreeViewColumn* col, GtkColumnOptions o);

    void setup_columns_editable (void);
    void setup_columns_normal (void);
    void toggle_edit_context (void);

    std::string get_format (void);
    void set_format (std::string const& format_str);
};

/* ---------------------------------------------------------------- */

inline
GtkColumnOptions::GtkColumnOptions (void)
{
  this->removable = true;
  this->clickable = true;
  this->reorderable = true;
}

inline
GtkColumnOptions::GtkColumnOptions (bool rem, bool click, bool reorder,
    Glib::RefPtr<Gdk::Pixbuf> icon)
{
  this->removable = rem;
  this->clickable = click;
  this->reorderable = reorder;
  this->icon = icon;
}

#endif /* GTK_COLUMNS_BASE_HEADER */
