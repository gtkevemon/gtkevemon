// This file is part of GtkEveMon.
//
// GtkEveMon is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// You should have received a copy of the GNU General Public License
// along with GtkEveMon. If not, see <http://www.gnu.org/licenses/>.

#ifndef MAIN_GUI_HEADER
#define MAIN_GUI_HEADER

#include <vector>

#include <gtkmm.h>

#include "util/conf.h"
#include "bits/character.h"
#include "bits/characterlist.h"
#include "bits/updater.h"
#include "gtkinfodisplay.h"
#include "gtkserver.h"

/* Refresh the list of servers every this milli seconds. */
#define MAINGUI_SERVER_REFRESH 600000
/* Update the EVE time and the local time this milli seconds. */
#define MAINGUI_TIME_UPDATE 1000
/* Update the tooltip for the tray icon this milli seconds. */
#define MAINGUI_TOOLTIP_UPDATE 30000
/* Update the window title this milli seconds. */
#define MAINGUI_WINDOWTITLE_UPDATE 5000

class MainGui : public Gtk::Window
{
  private:
    ConfValuePtr conf_windowtitle;
    ConfValuePtr conf_detailed_tooltip;
    Updater* updater;
    std::vector<GtkServer*> gtkserver;
    Glib::RefPtr<Gtk::ActionGroup> actions;
    Glib::RefPtr<Gtk::UIManager> uiman;
    Glib::RefPtr<Gtk::StatusIcon> tray;
    Gtk::Notebook notebook;
    Gtk::Label evetime_label;
    Gtk::Label localtime_label;
    GtkInfoDisplay info_display;
    bool iconified;

  private:
    /* Misc helpers. */
    void init_from_charlist (void);
    void add_character (CharacterPtr character);
    void remove_character (std::string char_id);

    void on_pages_changed (Widget* page, guint page_num);
    void on_pages_switched (Widget* page, guint page_num);
    void on_data_files_changed (void);
    void on_data_files_unchanged (void);
    void check_if_no_pages (void);

    /* Update handlers. */
    bool update_servers (void);
    bool refresh_servers (void);
    bool update_time (void);
    bool update_tooltip (void);
    bool update_windowtitle (void);
    void update_char_name (std::string char_id);

    /* Actions. */
    void setup_profile (void);
    void configuration (void);
    void about_dialog (void);
    void version_checker (void);
    void launch_eve (void);
    void create_skillplan (void);
    void view_xml_source (void);
    void export_char_info (void);
    void close (void);

    /* Window state and tray icon. */
    bool on_delete_event (GdkEventAny* event);
    bool on_window_state_event (GdkEventWindowState* event);
    void on_tray_icon_clicked (void);
    void create_tray_icon (void);
    void destroy_tray_icon (void);
    void tray_popup_menu (guint button, guint32 activate_time);
    void update_tray_settings (void);

  public:
    MainGui (void);
    ~MainGui (void);
};

/* ---------------------------------------------------------------- */

inline
MainGui::~MainGui (void)
{
}

#endif /* MAIN_GUI_HEADER */
