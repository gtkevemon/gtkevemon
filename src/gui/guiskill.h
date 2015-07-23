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

#ifndef GUI_SKILL_HEADER
#define GUI_SKILL_HEADER

#include <gtkmm.h>

#include "winbase.h"

class GuiSkill : public WinBase
{
  private:
    Gtk::Label skill_name;
    Gtk::Label group_name;
    Gtk::Label skill_id;
    Gtk::Label group_id;
    Gtk::Label skill_attribs;
    Glib::RefPtr<Gtk::TextBuffer> desc_buffer;
    Gtk::TextView skill_desc;

  public:
    GuiSkill (void);
    void set_skill (int skill_id);
};

#endif /* GUI_SKILL_HEADER */
