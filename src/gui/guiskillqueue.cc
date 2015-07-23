// This file is part of GtkEveMon.
//
// GtkEveMon is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// You should have received a copy of the GNU General Public License
// along with GtkEveMon. If not, see <http://www.gnu.org/licenses/>.

#include <gtkmm.h>

#include "gtkdefines.h"
#include "guiskillqueue.h"

GuiSkillQueue::GuiSkillQueue (CharacterPtr character)
{
  Gtk::Frame* main_frame = MK_FRAME0;
  main_frame->add(this->queue);

  Gtk::Button* close_but = MK_BUT0;
  close_but->set_image_from_icon_name("window-close", Gtk::ICON_SIZE_BUTTON);

  Gtk::Box* button_box = MK_HBOX(5);
  button_box->pack_start(*MK_HSEP, true, true, 0);
  button_box->pack_start(*close_but, false, false, 0);

  Gtk::Box* main_box = MK_VBOX(5);
  main_box->set_border_width(5);
  main_box->pack_start(*main_frame, true, true, 0);
  main_box->pack_start(*button_box, false, false, 0);

  close_but->signal_clicked().connect(sigc::mem_fun
      (*this, &WinBase::close));

  this->add(*main_box);
  this->set_default_size(450, 300);
  this->set_title("Training queue - GtkEveMon");
  this->show_all();

  this->queue.set_character(character);
  this->queue.refresh();
}
