// This file is part of GtkEveMon.
//
// GtkEveMon is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// You should have received a copy of the GNU General Public License
// along with GtkEveMon. If not, see <http://www.gnu.org/licenses/>.

#ifndef GUI_EVE_LAUNCHER
#define GUI_EVE_LAUNCHER

#include <string>
#include <vector>

#include "winbase.h"

class GuiEveLauncher : public WinBase
{
  private:
    void run_command (std::string const& cmd);

  protected:
    GuiEveLauncher (std::vector<std::string> const& cmds);

  public:
    static Gtk::Window* launch_eve (void);
};

#endif /* GUI_EVE_LAUNCHER */
