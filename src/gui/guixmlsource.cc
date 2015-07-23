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
#include "guixmlsource.h"

GuiXmlSource::GuiXmlSource (void)
{
  Gtk::Button* close_but = MK_BUT("Close");
  Gtk::Box* button_box = MK_HBOX(5);
  button_box->pack_start(*MK_HSEP, true, true, 0);
  button_box->pack_start(*close_but, false, false, 0);

  Gtk::Box* main_box = MK_VBOX(5);
  main_box->set_border_width(5);
  main_box->pack_start(this->notebook, true, true, 0);
  main_box->pack_start(*button_box, false, false, 0);

  close_but->signal_clicked().connect(sigc::mem_fun(*this, &WinBase::close));

  this->add(*main_box);
  this->set_default_size(500, 600);
  this->set_title("XML Source - GtkEveMon");
  this->show_all();
}

/* ---------------------------------------------------------------- */

void
GuiXmlSource::append (HttpDataPtr data, std::string const& title)
{
  Glib::RefPtr<Gtk::TextBuffer> buffer = Gtk::TextBuffer::create();

  if (data.get() != 0 && !data->data.empty())
    buffer->set_text(&data->data[0]);
  else
    buffer->set_text("There is no data available!");

  Gtk::TextView* textview = Gtk::manage(new Gtk::TextView(buffer));
  textview->set_editable(false);
  Gtk::ScrolledWindow* scwin = MK_SCWIN;
  scwin->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_ALWAYS);
  scwin->set_border_width(5);
  scwin->set_shadow_type(Gtk::SHADOW_ETCHED_IN);
  scwin->add(*textview);
  scwin->show_all();
  this->notebook.append_page(*scwin, title);
}
