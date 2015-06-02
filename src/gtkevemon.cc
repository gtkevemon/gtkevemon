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

#include <csignal> // for ::signal()
#include <cstdlib> // for EXIT_SUCCESS
#include <gtkmm/main.h>

#include "api/evetime.h"
#include "bits/argumentsettings.h"
#include "bits/serverlist.h"
#include "bits/config.h"
#include "bits/server.h"
#include "bits/updater.h"
#include "gui/imagestore.h"
#include "gui/maingui.h"

void
signal_received (int /*signum*/)
{
  Gtk::Main::quit();
}

/* ---------------------------------------------------------------- */

int
main (int argc, char* argv[])
{
#ifdef WIN32
  if (!Glib::thread_supported())
    Glib::thread_init();
#endif

  Gtk::Main kit(&argc, &argv);
  ArgumentSettings::init(argc, argv);
  Config::init_defaults();
  Config::init_config_path();
  Config::init_user_config();

  ImageStore::init();

  Updater::check_data_files();

  ServerList::init_from_config();
  EveTime::init_from_config();

  std::signal(SIGINT, signal_received);
  std::signal(SIGTERM, signal_received);

  {
    MainGui gui;
    kit.run();
  }

  EveTime::store_to_config();
  ServerList::unload();
  ImageStore::unload();

  Config::unload();

  return EXIT_SUCCESS;
}
