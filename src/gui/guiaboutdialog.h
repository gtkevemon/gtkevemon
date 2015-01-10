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

#ifndef GUI_ABOUT_DIALOG_HEADER
#define GUI_ABOUT_DIALOG_HEADER

#include "net/asynchttp.h"
#include "winbase.h"

class GuiAboutDialog : public WinBase
{
  public:
    GuiAboutDialog (void);
};

#endif /* GUI_ABOUT_DIALOG_HEADER */
