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

#ifndef GTK_INFO_DISPLAY
#define GTK_INFO_DISPLAY

#include <string>

#include <gtkmm.h>

#include "winbase.h"

enum InfoItemType
{
  INFO_NOTIFICATION,
  INFO_WARNING,
  INFO_ERROR
};

enum InfoDisplayStyle
{
  INFO_STYLE_NONE,
  INFO_STYLE_TOP_HSEP,
  INFO_STYLE_FRAMED
};

/* ---------------------------------------------------------------- */

class InfoItem
{
  public:
    time_t time;
    InfoItemType type;
    std::string message;
    std::string details;

    InfoItem (void);
    InfoItem (InfoItemType type, std::string const& message);
    InfoItem (InfoItemType type, std::string const& message,
        std::string const& details);
};

/* ---------------------------------------------------------------- */

class GtkInfoDisplay : public Gtk::Box
{
  private:
    std::vector<InfoItem> log;
    Gtk::Image icon;
    Gtk::Label text;
    Gtk::Button info_but;

  protected:
    void show_info_log (void);

  public:
    GtkInfoDisplay (InfoDisplayStyle style);

    void append (InfoItemType type, std::string const& message);
    void append (InfoItemType type, std::string const& message,
        std::string const& details);
    void append (InfoItem const& item);
};

/* ---------------------------------------------------------------- */

class GuiInfoDisplayLog : public WinBase
{
  private:
    std::vector<InfoItem> log;
    std::size_t current_item;
    Gtk::Button prev_but;
    Gtk::Button next_but;
    Gtk::Label message;
    Glib::RefPtr<Gtk::TextBuffer> text_buffer;
    Gtk::TextView text_view;

  protected:
    void show_info_item (InfoItem const& item);
    void on_prev_clicked (void);
    void on_next_clicked (void);

  public:
    GuiInfoDisplayLog (std::vector<InfoItem> const& log);
};

/* ---------------------------------------------------------------- */

inline
InfoItem::InfoItem (void)
{
  this->time = ::time(0);
  this->type = INFO_ERROR;
}

inline
InfoItem::InfoItem (InfoItemType type, std::string const& message)
{
  this->time = ::time(0);
  this->type = type;
  this->message = message;
}

inline
InfoItem::InfoItem (InfoItemType type, std::string const& message,
    std::string const& details)
{
  this->time = ::time(0);
  this->type = type;
  this->message = message;
  this->details = details;
}

inline void
GtkInfoDisplay::append (InfoItemType type, std::string const& message)
{
  this->append(InfoItem(type, message));
}

inline void
GtkInfoDisplay::append (InfoItemType type, std::string const& message,
    std::string const& details)
{
  this->append(InfoItem(type, message, details));
}

#endif /* GTK_INFO_DISPLAY */
