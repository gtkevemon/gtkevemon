// This file is part of GtkEveMon.
//
// GtkEveMon is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// You should have received a copy of the GNU General Public License
// along with GtkEveMon. If not, see <http://www.gnu.org/licenses/>.

#include <cerrno>
#include <cstring>
#include <iostream>
#include <fstream>

#include <gtkmm.h>

#include "api/evetime.h"
#include "util/helpers.h"
#include "util/os.h"
#include "bits/config.h"
#include "gtkdefines.h"
#include "imagestore.h"
#include "guiconfiguration.h"
#include "guiupdater.h"

GuiUpdater::GuiUpdater (bool startup_mode)
{

  this->startup_mode = startup_mode;
  this->is_updated = false;
  this->download_error = false;
  if (startup_mode)
    this->close_but = MK_BUT("Quit");
  else
    this->close_but = MK_BUT("Close");
  this->update_but = MK_BUT("Refresh");

  Gtk::Button* config_but = MK_BUT0;
  config_but->set_image_from_icon_name("preferences-system", Gtk::ICON_SIZE_BUTTON);

  Gtk::Label* info_label = MK_LABEL0;
  info_label->set_line_wrap(true);
  info_label->set_halign(Gtk::ALIGN_START);
  if (startup_mode)
    info_label->set_text("Some data files are not available on your system. "
        "All of the following files are required for GtkEveMon to run. "
        "GtkEveMon is going to download these files now.");
  else
    info_label->set_text("With this dialog you can manually check for "
        "recent versions of the data files and keep them up-to-date. "
        "After downloading a new version, the application will exit.");

  this->frame_box = MK_VBOX(5);
  this->frame_box->set_border_width(5);
  this->frame_box->pack_start(*info_label, false, false, 0);
  this->frame_box->pack_start(*MK_HSEP, false, false, 0);
  this->frame_box->pack_end(this->downloader, false, false, 0);

  Gtk::Frame* main_frame = MK_FRAME0;
  main_frame->set_shadow_type(Gtk::SHADOW_OUT);
  main_frame->add(*frame_box);

  Gtk::Box* button_box = MK_HBOX(5);
  button_box->pack_start(*this->close_but, false, false, 0);
  button_box->pack_end(*this->update_but, false, false, 0);
  if (this->startup_mode)
    button_box->pack_start(*config_but, false, false, 0);

  Gtk::Box* main_vbox = MK_VBOX(5);
  main_vbox->pack_start(*main_frame, true, true, 0);
  main_vbox->pack_end(*button_box, false, false);

  Gtk::Image* logo_img = Gtk::manage(new Gtk::Image(ImageStore::aboutlogo));
  Gtk::Frame* logo_frame = MK_FRAME0;
  logo_frame->add(*logo_img);
  logo_frame->set_shadow_type(Gtk::SHADOW_IN);

  Gtk::Box* main_hbox = MK_HBOX(5);
  main_hbox->set_border_width(5);
  main_hbox->pack_start(*logo_frame, false, false, 0);
  main_hbox->pack_start(*main_vbox, true, true, 0);

  this->close_but->signal_clicked().connect(sigc::mem_fun
      (*this, &GuiUpdater::on_close_clicked));
  this->update_but->signal_clicked().connect(sigc::mem_fun
      (*this, &GuiUpdater::on_update_clicked));
  config_but->signal_clicked().connect(sigc::mem_fun
      (*this, &GuiUpdater::on_config_clicked));
  this->downloader.signal_download_done().connect(sigc::mem_fun
      (*this, &GuiUpdater::on_download_done));
  this->downloader.signal_all_downloads_done().connect
      (sigc::mem_fun(*this, &GuiUpdater::on_update_done));

  this->set_title("Data Files Updater - GtkEveMon");
  this->set_default_size(525, -1);
  this->add(*main_hbox);
  this->show_all();
  this->downloader.hide();

  this->files_box = MK_BOX0;
  this->rebuild_files_box();

  //if (startup_mode)
  //  this->on_update_clicked();
}

/* ---------------------------------------------------------------- */

GuiUpdater::~GuiUpdater (void)
{
  if (this->startup_mode)
    Gtk::Main::quit();

  if (this->startup_mode && !this->is_updated)
    std::exit(EXIT_SUCCESS);

  if (!this->startup_mode && this->is_updated)
    std::exit(EXIT_SUCCESS);
}

/* ---------------------------------------------------------------- */

void
GuiUpdater::rebuild_files_box (void)
{
    delete files_box;

    this->files_box = MK_BOX0;
    this->files_box->set_orientation(Gtk::ORIENTATION_VERTICAL);
    this->files_box->set_spacing(5);
    this->frame_box->pack_start(*files_box, false, false, 0);

  for (std::size_t i = 0; i < this->files.size(); ++i)
  {
    UpdaterDataFile const& file = this->files[i];
    bool file_exists = OS::file_exists(file.local_path.c_str());
    std::size_t file_size = OS::file_size(file.local_path.c_str());
    std::string info_size;
    if (file_exists)
        info_size = "Size: " + Helpers::get_string_from_float
            (static_cast<float>(file_size) / 1024.0f, 2) + " KB";
    else
        info_size = "File does not exist!";

    int last_updated = Config::conf.get_value("updater.last_update")->get_int();
    std::string last_updated_str;
    if (last_updated == 0)
        last_updated_str = "never";
    else
        last_updated_str = EveTime::get_local_time_string(last_updated, false);


    Gtk::Image* status_image = MK_IMG0;
    if (file_exists)
    {
        status_image->set_from_icon_name("gtk-yes", Gtk::ICON_SIZE_BUTTON);
    }
    else
    {
        status_image->set_from_icon_name("gtk-no", Gtk::ICON_SIZE_BUTTON);
    }

    Gtk::Label* filename_label = MK_LABEL("<b>" + file.file_name + "</b>");
    filename_label->set_use_markup(true);
    filename_label->set_halign(Gtk::ALIGN_START);

    Gtk::Label* size_label = MK_LABEL(info_size);
    size_label->set_halign(Gtk::ALIGN_START);

    Gtk::Label* last_update = MK_LABEL("Last Update");
    last_update->set_halign(Gtk::ALIGN_END);
    Gtk::Label* update_date = MK_LABEL(last_updated_str);
    update_date->set_halign(Gtk::ALIGN_END);

    Gtk::Table* entry = MK_TABLE(2, 3);
    entry->set_col_spacings(10);
    entry->attach(*status_image, 0, 1, 0, 2, Gtk::SHRINK | Gtk::FILL);
    entry->attach(*filename_label, 1, 2, 0, 1, Gtk::SHRINK | Gtk::FILL);
    entry->attach(*size_label, 1, 2, 1, 2, Gtk::SHRINK | Gtk::FILL);
    entry->attach(*last_update, 2, 3, 0, 1, Gtk::EXPAND | Gtk::FILL);
    entry->attach(*update_date, 2, 3, 1, 2, Gtk::EXPAND | Gtk::FILL);

    this->files_box->pack_start(*entry, false, false, 0);
    this->files_box->pack_start(*MK_HSEP, false, false, 0);
  }

  this->files_box->show_all();
}

/* ---------------------------------------------------------------- */

void
GuiUpdater::on_close_clicked (void)
{
  /* Backup startup mode flag because close will delete members. */
  bool startup_mode = this->startup_mode;

  this->close();

  if (startup_mode)
    std::exit(EXIT_SUCCESS);
}

/* ---------------------------------------------------------------- */

void
GuiUpdater::on_update_clicked (void)
{
  this->rebuild_files_box();
  this->update_but->set_sensitive(false);

  if (!is_updated)
  {
    this->download_error = false;
    for (std::size_t i = 0; i < this->files.size(); ++i)
    {
      UpdaterDataFile const& file = this->files[i];
      DownloadItem dl;
      dl.name = file.file_name;
      dl.host = file.server_host;
      dl.path = file.server_path;
      dl.is_api_call = true;
      this->downloader.append_download(dl);
    }
    this->downloader.show();
    this->downloader.start_downloads();
  }
  else
  {
    if (this->startup_mode)
      this->close();
    else
      Gtk::Main::quit();
  }
}

/* ---------------------------------------------------------------- */

void
GuiUpdater::on_config_clicked (void)
{
  GuiConfiguration* dialog = new GuiConfiguration();
  dialog->set_transient_for(*this);
}

/* ---------------------------------------------------------------- */

void
GuiUpdater::on_download_done (DownloadItem dl, AsyncHttpData data)
{
  /* Download successful? */
  if (data.data.get() == 0 || data.data->http_code != 200)
  {
    std::cout << "Error: The download for " << dl.name
        << " failed: " << data.exception << std::endl;

    Gtk::MessageDialog md("Download failed!",
        false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);
    md.set_secondary_text("The download for " + dl.name
        + " failed.\n" + data.exception);
    md.set_transient_for(*this);
    md.run();

    this->download_error = true;
    return;
  }

  /* File corresponding updater file info. */
  UpdaterDataFile file;
  for (std::size_t i = 0; i < this->files.size(); ++i)
  {
    if (this->files[i].file_name == dl.name)
    {
      file = files[i];
      break;
    }
  }
  if (file.file_name.empty())
  {
    std::cout << "Error: The download could not be associated" << std::endl;

    Gtk::MessageDialog md("Download failed!",
        false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);
    md.set_transient_for(*this);
    md.run();

    this->download_error = true;
    return;
  }

  /* Check if file changed. */
  if (Updater::is_same_file(file.local_path, data.data))
    return;

  /* Write file to disk. */
  std::ofstream out(file.local_path.c_str());
  if (!out)
  {
    std::cout << "Error: Cannot write data file to disk." << std::endl;

    Gtk::MessageDialog md("Write to disk failed!",
        false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);
    md.set_secondary_text("The download for " + dl.name
        + " succeeded, but the file cannot be written to disk.\n\n"
        + std::string(::strerror(errno)));
    md.set_transient_for(*this);
    md.run();

    this->download_error = true;
    return;
  }
  out.write(&data.data->data[0], data.data->data.size());
  out.close();

  this->is_updated = true;
  this->rebuild_files_box();
}

/* ---------------------------------------------------------------- */

void
GuiUpdater::on_update_done (void)
{
  this->downloader.hide();
  this->update_but->set_sensitive(true);
  if (this->download_error)
    return;

  Updater::set_last_update_now();
  if (this->startup_mode)
  {
    this->update_but->set_sensitive(true);
    this->update_but->set_image_from_icon_name("media-playback-start", Gtk::ICON_SIZE_BUTTON);
    this->update_but->set_label("Continue");
    this->append_ui_info("Update successful.");
  }
  else if (this->is_updated)
  {
    this->close_but->set_sensitive(false);
    this->update_but->set_sensitive(true);
    this->update_but->set_image_from_icon_name("application-exit", Gtk::ICON_SIZE_BUTTON);
    this->update_but->set_label("Quit");

    this->append_ui_info(
        "Update successful. GtkEveMon needs to be\n"
        "restarted to reload the files. Please restart!");
  }
  else
  {
    this->append_ui_info(
        "The downloaded files match the local files.\n"
        "The update had no effect, try again later.");
  }
}

/* ---------------------------------------------------------------- */

void
GuiUpdater::append_ui_info (std::string const& message)
{
  Gtk::Label* error_label = MK_LABEL(message);
  error_label->set_line_wrap(true);
  error_label->set_halign(Gtk::ALIGN_START);

  Gtk::Box* error_box = MK_HBOX(5);
  Gtk::Image* info_image = MK_IMG0;
  info_image->set_from_icon_name("dialog-information",
      Gtk::ICON_SIZE_DIALOG);
  error_box->set_border_width(5);
  error_box->pack_start(*info_image, false, false, 0);
  error_box->pack_start(*error_label, true, true, 0);

  this->files_box->pack_start(*error_box, false, false, 0);
  this->files_box->show_all();
}
