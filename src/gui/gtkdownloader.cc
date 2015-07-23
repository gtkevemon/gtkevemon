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

#include "util/helpers.h"
#include "net/http.h"
#include "bits/config.h"
#include "gtkdefines.h"
#include "gtkdownloader.h"

GtkDownloader::GtkDownloader (void)
{
  this->asynchttp = 0;
  this->progressbar.set_text(" ");

  Gtk::Box* filename_box = MK_HBOX(5);
  filename_box->pack_start(*MK_LABEL("Downloading:"), false, false, 0);
  filename_box->pack_start(this->filename_label, false, false, 0);

  Gtk::Button* cancel_but = MK_BUT0;
  cancel_but->set_image_from_icon_name("gtk-cancel", Gtk::ICON_SIZE_MENU);
  cancel_but->set_relief(Gtk::RELIEF_NONE);

  Gtk::Table* main_table = MK_TABLE(2, 1);
  main_table->set_col_spacings(5);
  main_table->set_row_spacings(1);
  main_table->attach(*filename_box, 0, 2, 0, 1, Gtk::EXPAND | Gtk::FILL);
  main_table->attach(this->progressbar, 0, 1, 1, 2, Gtk::EXPAND | Gtk::FILL);
  //main_table->attach(*cancel_but, 1, 2, 1, 2, Gtk::SHRINK | Gtk::FILL);

  cancel_but->signal_clicked().connect(sigc::mem_fun
      (*this, &GtkDownloader::on_cancel_clicked));

  this->on_update_event();
  this->set_shadow_type(Gtk::SHADOW_NONE);
  this->add(*main_table);
}

/* ---------------------------------------------------------------- */

void
GtkDownloader::start_downloads (void)
{
  this->start_next_download();

  Glib::signal_timeout().connect(sigc::mem_fun(*this,
      &GtkDownloader::on_update_event), 100);
}

/* ---------------------------------------------------------------- */

void
GtkDownloader::start_next_download (void)
{
  if (this->downloads.size() == 0)
    return;

  DownloadItem dli = this->downloads.front();

  this->filename_label.set_text("<b>" + dli.name + "</b>");
  this->filename_label.set_use_markup(true);

  this->asynchttp = AsyncHttp::create();
  this->asynchttp->set_host(dli.host);
  this->asynchttp->set_path(dli.path);
  Config::setup_http(this->asynchttp, dli.is_api_call);

  this->asynchttp->signal_done().connect(sigc::mem_fun
      (*this, &GtkDownloader::on_download_complete));
  this->asynchttp->async_request();
}

/* ---------------------------------------------------------------- */

void
GtkDownloader::cancel_downloads (void)
{
}

/* ---------------------------------------------------------------- */

void
GtkDownloader::on_cancel_clicked (void)
{
}

/* ---------------------------------------------------------------- */

void
GtkDownloader::on_download_complete (AsyncHttpData data)
{
  this->on_update_event();

  /* Invalidate HTTP fetcher. */
  this->asynchttp = 0;

  DownloadItem dli = this->downloads.front();
  this->downloads.erase(this->downloads.begin());

  /* Transport the download to the outer world. */
  this->sig_download_done.emit(dli, data);

  if (this->downloads.empty())
    this->sig_all_downloads_done.emit();
  else
    this->start_next_download();
}

/* ---------------------------------------------------------------- */

bool
GtkDownloader::on_update_event (void)
{
  if (this->asynchttp == 0)
    return true;

  if (this->downloads.empty())
  {
    this->filename_label.set_text("");
    this->progressbar.set_text("0.0%");
    this->progressbar.set_fraction(0.0);

    return !this->downloads.empty();
  }

  size_t bytes_total = this->asynchttp->get_bytes_total();
  size_t bytes_read = this->asynchttp->get_bytes_read();

  std::string bytes_read_str = Helpers::get_string_from_float
      ((float)bytes_read / 1024.0f, 0);

  if (bytes_total == 0)
  {
    this->progressbar.set_text(bytes_read_str + " KB");
    this->progressbar.set_fraction(0.0);
  }
  else
  {
    float percent = 100.0f * (float)bytes_read / (float)bytes_total;
    std::string percent_str = Helpers::get_string_from_float(percent, 1);
    this->progressbar.set_text(bytes_read_str + "KB - " + percent_str + "%");
    this->progressbar.set_fraction(percent / 100.0f);
  }

  return true;
}
