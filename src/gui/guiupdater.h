// This file is part of GtkEveMon.
//
// GtkEveMon is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// You should have received a copy of the GNU General Public License
// along with GtkEveMon. If not, see <http://www.gnu.org/licenses/>.

#ifndef GUI_UPDATER_HEADER
#define GUI_UPDATER_HEADER

#include <gtkmm.h>

#include "bits/updater.h"
#include "net/asynchttp.h"
#include "gtkdownloader.h"
#include "winbase.h"

class GuiUpdater : public WinBase, public UpdaterBase
{
  private:
    bool startup_mode;
    bool is_updated;
    bool download_error;

    GtkDownloader downloader;
    Gtk::Button* close_but;
    Gtk::Button* update_but;
    Gtk::Box* frame_box;
    Gtk::Box* files_box;

  protected:
    void rebuild_files_box (void);
    void on_update_clicked (void);
    void on_close_clicked (void);
    void on_config_clicked (void);
    void on_download_done (DownloadItem dl, AsyncHttpData data);
    void on_update_done (void);
    void append_ui_info (std::string const& message);

  public:
    GuiUpdater (bool startup_mode);
    ~GuiUpdater (void);
};

#endif /* GUI_UPDATER_HEADER */
