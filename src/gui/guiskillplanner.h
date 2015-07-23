// This file is part of GtkEveMon.
//
// GtkEveMon is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// You should have received a copy of the GNU General Public License
// along with GtkEveMon. If not, see <http://www.gnu.org/licenses/>.

#ifndef GUI_SKILL_PLANNER_HEADER
#define GUI_SKILL_PLANNER_HEADER

#include <gtkmm.h>

#include "bits/character.h"
#include "winbase.h"
#include "gtkitemdetails.h"
#include "gtkitembrowser.h"
#include "gtktrainingplan.h"

class GuiSkillPlanner : public WinBase
{
  private:
    GtkSkillBrowser skill_browser;
    GtkCertBrowser cert_browser;
    GtkItemDetails details_gui;
    GtkTrainingPlan plan_gui;

    /* Character stuff. */
    CharacterPtr character;

    /* Misc. */
    Gtk::Notebook details_nb;
    Gtk::Notebook browser_nb;
    Gtk::HPaned main_pane;

    void on_element_selected (ApiElement const* elem);
    void on_element_activated (ApiElement const* elem);
    void on_planning_requested (ApiElement const* skill, int level);

    void init_from_config (void);
    void store_to_config (void);

  public:
    GuiSkillPlanner (void);
    ~GuiSkillPlanner (void);

    void set_character (CharacterPtr character);
};

#endif /* GUI_SKILL_PLANNER_HEADER */
