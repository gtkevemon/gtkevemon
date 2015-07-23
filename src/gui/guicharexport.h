// This file is part of GtkEveMon.
//
// GtkEveMon is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// You should have received a copy of the GNU General Public License
// along with GtkEveMon. If not, see <http://www.gnu.org/licenses/>.

#ifndef GUI_CHAR_EXPORT_HEADER
#define GUI_CHAR_EXPORT_HEADER

#include <map>
#include <string>

#include <gtkmm.h>

#include "api/apicharsheet.h"
#include "winbase.h"

class GtkExportPane : public Gtk::Box
{
  private:
    Glib::RefPtr<Gtk::TextBuffer> text_buf;

  private:
    void on_save_clicked (void);

  public:
    GtkExportPane (void);
    void set_text (Glib::ustring const& text);
};

/* ---------------------------------------------------------------- */

class GuiCharExport : public WinBase
{
  public:
    typedef std::map<std::string, ApiCharSheetSkill const*> SkillMap;
    typedef std::map<std::string, SkillMap> GroupMap;

  private:
    static void create_datamap (GroupMap& result, ApiCharSheetPtr charsheet);

  protected:
    void fill_eft (ApiCharSheetPtr charsheet, GtkExportPane* pane);
    void fill_bbc (ApiCharSheetPtr charsheet, GtkExportPane* pane);

  public:
    GuiCharExport (ApiCharSheetPtr charsheet);
};

#endif /* GUI_CHAR_EXPORT_HEADER */
