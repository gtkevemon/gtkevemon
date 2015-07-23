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
#include "util/thread.h"
#include "bits/serverlist.h"
#include "gtkserver.h"

class GtkServerChecker : public Thread
{
  private:
    ServerPtr server;

  protected:
    void* run (void);

  public:
    GtkServerChecker (ServerPtr server);
};

/* ---------------------------------------------------------------- */

GtkServerChecker::GtkServerChecker (ServerPtr server)
{
  this->server = server;
}

/* ---------------------------------------------------------------- */

void*
GtkServerChecker::run (void)
{
  try
  {
    this->server->refresh();
  }
  catch (Exception& e)
  {
    std::cout << "Error refeshing server: " << e << std::endl;
  }

  delete this;
  return 0;
}

/* ================================================================ */

GtkServer::GtkServer (ServerPtr server)
  : Gtk::Table(2, 3, false)
{
  this->server = server;
  this->status_desc.set_text("Status:");
  this->status.set_text("Fetching");

  this->set_col_spacings(5);
  this->set_status_icon("view-refresh");
  this->status_but.set_relief(Gtk::RELIEF_NONE);

  Gtk::Label* label_server = Gtk::manage(new Gtk::Label("Server:"));

  label_server->set_halign(Gtk::ALIGN_START);
  this->status_desc.set_halign(Gtk::ALIGN_START);

  this->name.set_halign(Gtk::ALIGN_END);
  this->status.set_halign(Gtk::ALIGN_END);

  this->name.set_text(server->get_name());

  this->attach(this->status_but, 0, 1, 0, 2, Gtk::FILL, Gtk::FILL);
  this->attach(*label_server, 1, 2, 0, 1, Gtk::FILL, Gtk::FILL);
  this->attach(this->status_desc, 1, 2, 1, 2, Gtk::FILL, Gtk::FILL);
  this->attach(this->name, 2, 3, 0, 1, Gtk::FILL, Gtk::FILL);
  this->attach(this->status, 2, 3, 1, 2, Gtk::FILL, Gtk::FILL);

  this->status_but.signal_clicked().connect
      (sigc::mem_fun(*this, &GtkServer::force_refresh));
  this->server->signal_updated().connect
      (sigc::mem_fun(*this, &GtkServer::update));

  this->update();
  this->show_all();
}

/* ---------------------------------------------------------------- */

void
GtkServer::update (void)
{
  if (server->is_refreshing() || server->get_players() == -1)
  {
    this->set_status_icon("view-refresh");
  }
  else if (server->is_online())
  {
    this->set_status_icon("gtk-yes");
    this->status_desc.set_text("Players:");

    if (server->get_players() == -2)
      this->status.set_text("Unknown");
    else
      this->status.set_text(Helpers::get_dotted_str_from_int
          (this->server->get_players()));
  }
  else
  {
    this->set_status_icon("gtk-no");
    this->status_desc.set_text("Status:");
    this->status.set_text("Offline");
  }
}

/* ---------------------------------------------------------------- */

void
GtkServer::force_refresh (void)
{
  GtkServerChecker* sc = new GtkServerChecker(this->server);
  sc->pt_create();
  this->set_status_icon("view-refresh");
}

/* ---------------------------------------------------------------- */

void
GtkServer::set_status_icon (const Glib::ustring icon_name)
{
  this->status_but.set_image_from_icon_name(icon_name, Gtk::ICON_SIZE_BUTTON);
}
