// This file is part of GtkEveMon.
//
// GtkEveMon is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// You should have received a copy of the GNU General Public License
// along with GtkEveMon. If not, see <http://www.gnu.org/licenses/>.

#ifndef GTK_SERVER_HEADER
#define GTK_SERVER_HEADER

#include <gtkmm.h>

#include "bits/server.h"

class GtkServer : public Gtk::Table
{
  private:
    ServerPtr server;

    Gtk::Button status_but;
    Gtk::Label name;
    Gtk::Label status;
    Gtk::Label status_desc;

    void force_refresh (void);
    void set_status_icon (const Glib::ustring icon_name);

  public:
    GtkServer (ServerPtr server);
    void update (void);
};

#endif /* GTK_SERVER_HEADER */
