// This file is part of GtkEveMon.
//
// GtkEveMon is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// You should have received a copy of the GNU General Public License
// along with GtkEveMon. If not, see <http://www.gnu.org/licenses/>.

#ifndef GUI_SKILL_QUEUE_HEADER
#define GUI_SKILL_QUEUE_HEADER

#include "bits/character.h"
#include "winbase.h"
#include "gtkskillqueue.h"

class GuiSkillQueue : public WinBase
{
  private:
    GtkSkillQueue queue;

  public:
    GuiSkillQueue (CharacterPtr character);
};

#endif /* GUI_SKILL_QUEUE_HEADER */
