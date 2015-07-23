// This file is part of GtkEveMon.
//
// GtkEveMon is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// You should have received a copy of the GNU General Public License
// along with GtkEveMon. If not, see <http://www.gnu.org/licenses/>.

#ifndef GTK_PORTRAIT_HEADER
#define GTK_PORTRAIT_HEADER

#include <string>

#include <gdkmm.h>
#include <gtkmm.h>

#include "net/asynchttp.h"

/* The size of the portrait (pixels) in the GUI. */
#define PORTRAIT_SIZE 85

class GtkPortrait : public Gtk::EventBox
{
  private:
    Gtk::Image image;
    std::string char_id;
    sigc::connection http_request;

    bool fetch_from_gtkevemon_cache (void);
    void request_from_eve_online (void);
    void set_from_eve_online (AsyncHttpData result);
    bool on_button_press_myevent (GdkEventButton* event);
    std::string get_portrait_dir (void);
    std::string get_portrait_file (void);

    void cache_portrait (Glib::RefPtr<Gdk::Pixbuf> portrait);

  private:
    static Glib::RefPtr<Gdk::Pixbuf> create_from_file (std::string const& fn);
    static void save_to_file (Glib::RefPtr<Gdk::Pixbuf> pixbuf,
        std::string const& fn);

  public:
    GtkPortrait (void);
    GtkPortrait (std::string const& charid);
    ~GtkPortrait (void);

    void set (std::string const& charid);
    void set_enable_clicks (void);
    void update (void);
};

#endif /* GTK_PORTRAIT_HEADER */
