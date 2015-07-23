// This file is part of GtkEveMon.
//
// GtkEveMon is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// You should have received a copy of the GNU General Public License
// along with GtkEveMon. If not, see <http://www.gnu.org/licenses/>.

#ifndef IMAGE_STORE_HEADER
#define IMAGE_STORE_HEADER

#include <gdkmm.h>

class ImageStore
{
  private:
    static Glib::RefPtr<Gdk::Pixbuf> create_from_inline (guint8 const* data);

  public:
    static Glib::RefPtr<Gdk::Pixbuf> skill;
    static Glib::RefPtr<Gdk::Pixbuf> skillqueue;
    static Glib::RefPtr<Gdk::Pixbuf> certificate;
    static Glib::RefPtr<Gdk::Pixbuf> certificate_small;
    static Glib::RefPtr<Gdk::Pixbuf> applogo;
    static Glib::RefPtr<Gdk::Pixbuf> aboutlogo;
    static Glib::RefPtr<Gdk::Pixbuf> eveportrait;
    static Glib::RefPtr<Gdk::Pixbuf> columnconf[2];
    static Glib::RefPtr<Gdk::Pixbuf> skillicons[6];
    static Glib::RefPtr<Gdk::Pixbuf> skillstatus[8];
    static Glib::RefPtr<Gdk::Pixbuf> skilldeps[3];
    static Glib::RefPtr<Gdk::Pixbuf> skillplan[5];
    static Glib::RefPtr<Gdk::Pixbuf> certgrades[4];
    static Glib::RefPtr<Gdk::Pixbuf> certstatus[4];
    static Glib::RefPtr<Gdk::Pixbuf> menuicons[3];

    static Glib::RefPtr<Gdk::Pixbuf> skill_progress
        (unsigned int level, double percent);

    static void init (void);
    static void unload (void);
};

#endif /* IMAGE_STORE_HEADER */
