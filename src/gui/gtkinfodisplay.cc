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

#include "api/evetime.h"
#include "util/exception.h"
#include "gtkdefines.h"
#include "gtkhelpers.h"
#include "gtkinfodisplay.h"

GtkInfoDisplay::GtkInfoDisplay (InfoDisplayStyle style)
  : Gtk::Box(Gtk::ORIENTATION_VERTICAL, 0)
{
  this->text.set_halign(Gtk::ALIGN_START);

  this->info_but.set_image_from_icon_name("edit-find", Gtk::ICON_SIZE_MENU);
  this->info_but.set_relief(Gtk::RELIEF_NONE);
  this->info_but.set_tooltip_text("Detailed information");

  Gtk::Button* close_but = MK_BUT0;
  close_but->set_image_from_icon_name("window-close", Gtk::ICON_SIZE_MENU);
  close_but->set_relief(Gtk::RELIEF_NONE);
  close_but->set_tooltip_text("Hide message box");

  Gtk::Box* button_box = MK_HBOX(0);
  button_box->pack_start(this->info_but, false, false, 0);
  button_box->pack_start(*close_but, false, false, 0);

  Gtk::Box* item_box = MK_HBOX(5);
  item_box->pack_start(this->icon, false, false, 0);
  item_box->pack_start(this->text, true, true, 0);
  item_box->pack_start(*button_box, false, false, 0);

  switch (style)
  {
    default:
    case INFO_STYLE_NONE:
      this->pack_start(*item_box, false, false, 0);
      break;
    case INFO_STYLE_TOP_HSEP:
      this->pack_start(*MK_HSEP, false, false, 0);
      this->pack_start(*item_box, false, false, 0);
      break;
    case INFO_STYLE_FRAMED:
      Gtk::Frame* frame = MK_FRAME0;
      frame->set_shadow_type(Gtk::SHADOW_OUT);
      frame->add(*item_box);
      this->pack_start(*frame, false, false, 0);
      break;
  }

  close_but->signal_clicked().connect(sigc::mem_fun
      (*this, &Gtk::Widget::hide));
  this->info_but.signal_clicked().connect(sigc::mem_fun
      (*this, &GtkInfoDisplay::show_info_log));
}

/* ---------------------------------------------------------------- */

void
GtkInfoDisplay::append (InfoItem const& item)
{
  this->text.set_text(GtkHelpers::locale_to_utf8(item.message));
  this->text.set_use_markup(true);

  switch (item.type)
  {
    case INFO_NOTIFICATION:
      this->icon.set_from_icon_name("dialog-information", Gtk::ICON_SIZE_MENU);
      break;
    case INFO_WARNING:
      this->icon.set_from_icon_name("dialog-warning", Gtk::ICON_SIZE_MENU);
      break;
    default:
    case INFO_ERROR:
      this->icon.set_from_icon_name("dialog-error", Gtk::ICON_SIZE_MENU);
      break;
  }

  if (item.details.empty())
    this->info_but.set_sensitive(false);
  else
    this->info_but.set_sensitive(true);

  this->log.push_back(item);
  this->show();
}

/* ---------------------------------------------------------------- */

void
GtkInfoDisplay::show_info_log (void)
{
  GuiInfoDisplayLog* log = new GuiInfoDisplayLog(this->log);

  Gtk::Window* toplevel = (Gtk::Window*)this->get_toplevel();
  log->set_transient_for(*toplevel);

  this->hide();
}

/* ---------------------------------------------------------------- */

GuiInfoDisplayLog::GuiInfoDisplayLog (std::vector<InfoItem> const& log)
  : log(log),
    prev_but("_Back"),
    next_but("_Forward"),
    text_buffer(Gtk::TextBuffer::create()),
    text_view(text_buffer)
{
  if (log.size() == 0)
    throw Exception("Need log entries to display!");

  Gtk::Button* close_but = MK_BUT0;
  close_but->set_image_from_icon_name("window-close", Gtk::ICON_SIZE_BUTTON);

  Gtk::Box* button_box = MK_HBOX(5);
  button_box->pack_start(this->prev_but, false, false, 0);
  button_box->pack_start(this->next_but, false, false, 0);
  button_box->pack_start(*MK_HSEP, true, true, 0);
  button_box->pack_start(*close_but, false, false, 0);

  Gtk::ScrolledWindow* scwin = MK_SCWIN;
  scwin->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_ALWAYS);
  scwin->add(this->text_view);
  scwin->set_shadow_type(Gtk::SHADOW_ETCHED_IN);

  this->message.set_halign(Gtk::ALIGN_START);
  this->text_view.set_editable(false);
  this->text_view.set_wrap_mode(Gtk::WRAP_WORD);

  Gtk::Box* main_box = MK_VBOX(5);
  main_box->set_border_width(5);
  main_box->pack_start(this->message, false, false, 0);
  main_box->pack_start(*scwin, true, true, 0);
  main_box->pack_end(*button_box, false, false, 0);

  this->add(*main_box);
  this->set_title("Info Log - GtkEveMon");

  close_but->signal_clicked().connect(sigc::mem_fun(*this, &WinBase::close));
  this->prev_but.signal_clicked().connect(sigc::mem_fun
      (*this, &GuiInfoDisplayLog::on_prev_clicked));
  this->next_but.signal_clicked().connect(sigc::mem_fun
      (*this, &GuiInfoDisplayLog::on_next_clicked));

  this->set_default_size(400, 250);
  this->show_all();

  this->next_but.set_sensitive(false);
  if (this->log.size() <= 1)
    this->prev_but.set_sensitive(false);

  this->current_item = this->log.size() - 1;
  this->show_info_item(this->log.back());
}

/* ---------------------------------------------------------------- */

void
GuiInfoDisplayLog::show_info_item (InfoItem const& item)
{
  this->message.set_text("<b>"
      + GtkHelpers::locale_to_utf8(item.message) + "</b>");
  this->message.set_use_markup(true);
  this->text_buffer->set_text("Event time: "
      + EveTime::get_local_time_string(item.time, false)
      + "\n\n" + GtkHelpers::locale_to_utf8(item.details));
}

/* ---------------------------------------------------------------- */

void
GuiInfoDisplayLog::on_prev_clicked (void)
{
  if (this->current_item > 0)
    this->current_item -= 1;
  this->show_info_item(this->log[this->current_item]);

  if (this->current_item == 0)
    this->prev_but.set_sensitive(false);
  this->next_but.set_sensitive(true);
}

/* ---------------------------------------------------------------- */

void
GuiInfoDisplayLog::on_next_clicked (void)
{
  if (this->current_item < this->log.size() - 1)
    this->current_item += 1;
  this->show_info_item(this->log[this->current_item]);

  if (this->current_item == this->log.size() - 1)
    this->next_but.set_sensitive(false);
  this->prev_but.set_sensitive(true);
}
