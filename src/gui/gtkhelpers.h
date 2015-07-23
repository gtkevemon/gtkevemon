/*
 * This file is part of GtkEveMon.
 *
 * GtkEveMon is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * You should have received a copy of the GNU General Public License
 * along with GtkEveMon. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GTK_HELPERS_HEADER
#define GTK_HELPERS_HEADER

#include <glibmm.h>
#include <gtkmm.h>

#include "api/apiskilltree.h"
#include "api/apicharsheet.h"
#include "bits/character.h"

class GtkHelpers
{
  public:
    static void create_tooltip (Glib::RefPtr<Gtk::Tooltip> const& tooltip,
        ApiSkill const* skill, ApiCharSheetSkill* cskill = 0,
        CharacterPtr character = CharacterPtr());

    static void create_tooltip (Glib::RefPtr<Gtk::Tooltip> const& tooltip,
        ApiCert const* cert);

    static bool create_tooltip_from_view (int x, int y,
        Glib::RefPtr<Gtk::Tooltip> tip, Gtk::TreeView& view,
        Glib::RefPtr<Gtk::TreeModel> store,
        Gtk::TreeModelColumn<ApiElement const*> col);

    static Glib::ustring locale_to_utf8 (Glib::ustring const& opsys_string);
};

#endif /* GTK_HELPERS_HEADER */
