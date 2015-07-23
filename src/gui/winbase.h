// This file is part of GtkEveMon.
//
// GtkEveMon is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// You should have received a copy of the GNU General Public License
// along with GtkEveMon. If not, see <http://www.gnu.org/licenses/>.

#ifndef WINBASE_HEADER
#define WINBASE_HEADER

#include <gtkmm.h>

class WinBase : public Gtk::Window
{
  protected:
    virtual bool on_delete_event (GdkEventAny* event);

  public:
    virtual ~WinBase (void);
    virtual void close (void);
};

/* ---------------------------------------------------------------- */

inline
WinBase::~WinBase (void)
{
}

inline bool
WinBase::on_delete_event (GdkEventAny* /*event*/)
{
  this->close();
  return false;
}

inline void
WinBase::close (void)
{
  delete this;
}

#endif /* WINBASE_HEADER */
