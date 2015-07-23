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

#include "util/helpers.h"
#include "imagestore.h"
#include "gtkdefines.h"
#include "gtkplannerbase.h"

GtkElementContextMenu::GtkElementContextMenu (void)
{
  this->signal_selection_done().connect(sigc::mem_fun
      (*this, &GtkElementContextMenu::delete_me));
}

/* ================================================================ */

void
GtkSkillContextMenu::set_skill (ApiSkill const* skill, int min_level)
{
  this->skill = skill;

  Gtk::ImageMenuItem* to_level[5];
  for (unsigned int i = 0; i < 5; ++i)
  {
    Glib::RefPtr<Gdk::Pixbuf> icon;
    if ((int)i + 1 <= min_level)
      icon = ImageStore::skillicons[5];
    else
      icon = ImageStore::skillicons[1];

    to_level[i] = Gtk::manage(new Gtk::ImageMenuItem
        (*Gtk::manage(new Gtk::Image(icon)),
        "Train to level " + Helpers::get_string_from_uint(i + 1)));
    to_level[i]->signal_activate().connect(sigc::bind(sigc::mem_fun
        (*this, &GtkSkillContextMenu::on_training_requested), i + 1));
    this->append(*to_level[i]);
  }
  this->show_all();
}

/* ---------------------------------------------------------------- */

void
GtkSkillContextMenu::on_training_requested (int level)
{
  this->sig_planning_requested.emit(this->skill, level);
}

/* ================================================================ */

void
GtkCertContextMenu::set_cert (ApiCert const* cert)
{
  this->cert = cert;
  Gtk::ImageMenuItem* menuitem = Gtk::manage(new Gtk::ImageMenuItem
      (*MK_IMG_PB(ImageStore::certificate_small),
      "Train certificate"));
  menuitem->signal_activate().connect(sigc::mem_fun
      (*this, &GtkCertContextMenu::on_training_requested));
  this->append(*menuitem);
  this->show_all();
}

/* ---------------------------------------------------------------- */

void
GtkCertContextMenu::on_training_requested (void)
{
  this->sig_planning_requested.emit(this->cert, 0);
}

/* ================================================================ */

bool
GtkListViewHelper::on_button_press_event (GdkEventButton* event)
{
  bool return_value = this->Gtk::TreeView::on_button_press_event(event);
  this->sig_button_press_event.emit(event);
  return return_value;
}
