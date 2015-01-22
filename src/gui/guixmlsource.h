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

#ifndef GUI_XML_SOURCE_HEADER
#define GUI_XML_SOURCE_HEADER

#include <string>
#include <gtkmm/notebook.h>

#include "net/http.h"
#include "winbase.h"

class GuiXmlSource : public WinBase
{
  private:
    Gtk::Notebook notebook;

  public:
    GuiXmlSource (void);

    void append (HttpDataPtr data, std::string const& title);
    void append (const char *data, std::string const& title);
};

#endif /* GUI_XML_SOURCE_HEADER */
