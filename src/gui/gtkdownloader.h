// This file is part of GtkEveMon.
//
// GtkEveMon is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// You should have received a copy of the GNU General Public License
// along with GtkEveMon. If not, see <http://www.gnu.org/licenses/>.

#ifndef GTK_DOWNLOADER_HEADER
#define GTK_DOWNLOADER_HEADER

#include <string>
#include <vector>

#include <gtkmm.h>

#include "net/asynchttp.h"

/* The progress is updated every this milli seconds. */
#define GTK_DOWNLOADER_GUI_UPDATE 100

struct DownloadItem
{
  std::string name;
  std::string host;
  std::string path;
  bool is_api_call;

  DownloadItem (void);
};

/* ---------------------------------------------------------------- */

typedef sigc::signal<void, DownloadItem, AsyncHttpData> SignalDownloadDone;
typedef sigc::signal<void> SignalAllDownloadsDone;

class GtkDownloader : public Gtk::Frame
{
  private:
    std::vector<DownloadItem> downloads;
    SignalDownloadDone sig_download_done;
    SignalAllDownloadsDone sig_all_downloads_done;

    Gtk::Label filename_label;
    Gtk::ProgressBar progressbar;

    AsyncHttp* asynchttp;

  protected:
    void start_next_download (void);
    void on_cancel_clicked (void);
    void on_download_complete (AsyncHttpData data);
    bool on_update_event (void);

  public:
    GtkDownloader (void);

    void append_download (DownloadItem const& item);
    void start_downloads (void);
    void cancel_downloads (void);

    SignalDownloadDone& signal_download_done (void);
    SignalAllDownloadsDone& signal_all_downloads_done (void);
};

/* ---------------------------------------------------------------- */

inline
DownloadItem::DownloadItem (void)
{
}

inline void
GtkDownloader::append_download (DownloadItem const& item)
{
  this->downloads.push_back(item);
}

inline SignalDownloadDone&
GtkDownloader::signal_download_done (void)
{
  return this->sig_download_done;
}

inline SignalAllDownloadsDone&
GtkDownloader::signal_all_downloads_done (void)
{
  return this->sig_all_downloads_done;
}

#endif /* GTK_DOWNLOADER_HEADER */
