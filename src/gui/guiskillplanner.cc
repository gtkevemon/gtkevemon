// This file is part of GtkEveMon.
//
// GtkEveMon is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// You should have received a copy of the GNU General Public License
// along with GtkEveMon. If not, see <http://www.gnu.org/licenses/>.

#include <iostream>

#include <gtkmm.h>

#include "util/helpers.h"
#include "bits/config.h"
#include "imagestore.h"
#include "gtkportrait.h"
#include "gtkdefines.h"
#include "guiskillplanner.h"

GuiSkillPlanner::GuiSkillPlanner (void)
{
  this->skill_browser.set_border_width(5);
  this->cert_browser.set_border_width(5);

  this->browser_nb.append_page(this->skill_browser, "Skills");
  this->browser_nb.append_page(this->cert_browser, "Certificates");
  this->browser_nb.set_size_request(260, -1);

  this->details_nb.append_page(this->plan_gui, "Training plan");
  this->details_nb.append_page(this->details_gui, "Item details");

  Gtk::Button* close_but = MK_BUT("Close");
  Gtk::Box* button_hbox = MK_HBOX(5);
  button_hbox->pack_start(*MK_HSEP, true, true, 0);
  button_hbox->pack_start(*close_but, false, false, 0);

  Gtk::Box* details_panechild = MK_VBOX(5);
  details_panechild->pack_start(this->details_nb, true, true, 0);
  details_panechild->pack_start(*button_hbox, false, false, 0);

  this->main_pane.add1(this->browser_nb);
  this->main_pane.add2(*details_panechild);

  Gtk::Box* main_vbox = MK_VBOX(5);
  main_vbox->set_border_width(5);
  main_vbox->pack_start(this->main_pane, true, true, 0);

  /* Signals. */
  this->skill_browser.signal_element_selected().connect
      (sigc::mem_fun(*this, &GuiSkillPlanner::on_element_selected));
  this->skill_browser.signal_element_activated().connect
      (sigc::mem_fun(*this, &GuiSkillPlanner::on_element_activated));
  this->skill_browser.signal_planning_requested().connect
      (sigc::mem_fun(*this, &GuiSkillPlanner::on_planning_requested));
  this->cert_browser.signal_element_selected().connect
      (sigc::mem_fun(*this, &GuiSkillPlanner::on_element_selected));
  this->cert_browser.signal_element_activated().connect
      (sigc::mem_fun(*this, &GuiSkillPlanner::on_element_activated));
  this->cert_browser.signal_planning_requested().connect
      (sigc::mem_fun(*this, &GuiSkillPlanner::on_planning_requested));

  close_but->signal_clicked().connect(sigc::mem_fun(*this, &WinBase::close));
  this->plan_gui.signal_skill_activated().connect(sigc::mem_fun
      (*this, &GuiSkillPlanner::on_element_activated));
  this->details_gui.signal_planning_requested().connect(sigc::mem_fun
      (*this, &GuiSkillPlanner::on_planning_requested));

  this->add(*main_vbox);
  this->set_title("Skill browser - GtkEveMon");
  this->set_icon(ImageStore::applogo);
  //this->set_default_size(800, 550);
  this->init_from_config();
  this->show_all();
}

/* ---------------------------------------------------------------- */

GuiSkillPlanner::~GuiSkillPlanner (void)
{
  this->store_to_config();
}

/* ---------------------------------------------------------------- */

void
GuiSkillPlanner::init_from_config (void)
{
  ConfSectionPtr planner = Config::conf.get_section("planner");
  ConfValuePtr gui_dimension = planner->get_value("gui_dimension");
  ConfValuePtr pane_pos = planner->get_value("pane_position");

  StringVector dim_xy = Helpers::split_string(**gui_dimension, 'x');
  if (dim_xy.size() == 2)
  {
    int dim_x = Helpers::get_int_from_string(dim_xy[0]);
    int dim_y = Helpers::get_int_from_string(dim_xy[1]);
    this->set_default_size(dim_x, dim_y);
  }

  this->main_pane.set_position(pane_pos->get_int());
}

/* ---------------------------------------------------------------- */

void
GuiSkillPlanner::store_to_config (void)
{
  ConfSectionPtr planner = Config::conf.get_section("planner");
  ConfValuePtr gui_dimension = planner->get_value("gui_dimension");
  ConfValuePtr pane_pos = planner->get_value("pane_position");

  int dim_x = 0;
  int dim_y = 0;
  this->get_size(dim_x, dim_y);
  gui_dimension->set(Helpers::get_string_from_int(dim_x) + "x"
      + Helpers::get_string_from_int(dim_y));

  pane_pos->set(this->main_pane.get_position());
}

/* ---------------------------------------------------------------- */

void
GuiSkillPlanner::set_character (CharacterPtr character)
{
  this->character = character;
  this->details_gui.set_character(character);
  this->plan_gui.set_character(character);
  /* FIXME: Pass character to browsers to register changed signal? */
  this->skill_browser.set_character(character->cs);
  this->cert_browser.set_character(character->cs);
  this->set_title(character->get_char_name() + " - GtkEveMon");
}

/* ---------------------------------------------------------------- */

void
GuiSkillPlanner::on_element_selected (ApiElement const* elem)
{
  this->details_gui.set_element(elem);
}

/* ---------------------------------------------------------------- */

void
GuiSkillPlanner::on_element_activated (ApiElement const* elem)
{
  this->details_nb.set_current_page(1);
  this->details_gui.set_element(elem);
}

/* ---------------------------------------------------------------- */

void
GuiSkillPlanner::on_planning_requested (ApiElement const* elem, int level)
{
  this->details_nb.set_current_page(0);
  this->plan_gui.append_element(elem, level);
}
