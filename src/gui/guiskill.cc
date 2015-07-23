// This file is part of GtkEveMon.
//
// GtkEveMon is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// You should have received a copy of the GNU General Public License
// along with GtkEveMon. If not, see <http://www.gnu.org/licenses/>.

#include <gtkmm.h>

#include "util/helpers.h"
#include "api/apiskilltree.h"
#include "gtkdefines.h"
#include "guiskill.h"

GuiSkill::GuiSkill (void)
  : desc_buffer(Gtk::TextBuffer::create()), skill_desc(desc_buffer)
{
  this->skill_desc.set_wrap_mode(Gtk::WRAP_WORD);
  this->skill_desc.set_editable(false);
  this->skill_name.set_selectable(true);
  this->skill_name.set_halign(Gtk::ALIGN_START);
  this->group_name.set_selectable(true);
  this->group_name.set_halign(Gtk::ALIGN_START);
  this->skill_attribs.set_selectable(true);
  this->skill_attribs.set_halign(Gtk::ALIGN_START);
  this->skill_id.set_halign(Gtk::ALIGN_END);
  this->group_id.set_halign(Gtk::ALIGN_END);

  Gtk::Label* skill_name_desc = MK_LABEL("Skill name:");
  Gtk::Label* group_name_desc = MK_LABEL("Skill group:");
  Gtk::Label* skill_attribs_desc = MK_LABEL("Attributes:");
  skill_name_desc->set_halign(Gtk::ALIGN_START);
  group_name_desc->set_halign(Gtk::ALIGN_START);
  skill_attribs_desc->set_halign(Gtk::ALIGN_START);

  Gtk::Table* info_table = MK_TABLE(3, 3);
  info_table->set_col_spacings(5);
  info_table->set_row_spacings(5);
  info_table->attach(*skill_name_desc, 0, 1, 0, 1, Gtk::FILL, Gtk::SHRINK);
  info_table->attach(*group_name_desc, 0, 1, 1, 2, Gtk::FILL, Gtk::SHRINK);
  info_table->attach(*skill_attribs_desc, 0, 1, 2, 3, Gtk::FILL, Gtk::SHRINK);
  info_table->attach(this->skill_name, 1, 2, 0, 1,
      Gtk::FILL|Gtk::EXPAND, Gtk::SHRINK);
  info_table->attach(this->group_name, 1, 2, 1, 2,
      Gtk::FILL|Gtk::EXPAND, Gtk::SHRINK);
  info_table->attach(this->skill_attribs, 1, 3, 2, 3,
      Gtk::FILL|Gtk::EXPAND, Gtk::SHRINK);
  info_table->attach(this->skill_id, 2, 3, 0, 1, Gtk::FILL);
  info_table->attach(this->group_id, 2, 3, 1, 2, Gtk::FILL);

  Gtk::Box* desc_separator = MK_HBOX(5);
  desc_separator->pack_start(*MK_HSEP, true, true, 0);
  desc_separator->pack_start(*MK_LABEL("Description"), false, false, 0);
  desc_separator->pack_start(*MK_HSEP, true, true, 0);

  Gtk::ScrolledWindow* desc_scwin = Gtk::manage(new Gtk::ScrolledWindow);
  desc_scwin->set_shadow_type(Gtk::SHADOW_ETCHED_IN);
  desc_scwin->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_ALWAYS);
  desc_scwin->add(this->skill_desc);

  Gtk::Box* skill_vbox = MK_VBOX(5);
  skill_vbox->set_border_width(5);
  skill_vbox->pack_start(*info_table, false, false, 0);
  skill_vbox->pack_start(*desc_separator, false, false, 0);
  skill_vbox->pack_start(*desc_scwin, true, true, 0);

  Gtk::Frame* info_frame = MK_FRAME0;
  info_frame->set_shadow_type(Gtk::SHADOW_OUT);
  info_frame->add(*skill_vbox);

  Gtk::Button* close_but = MK_BUT0;
  close_but->set_image_from_icon_name("window-close", Gtk::ICON_SIZE_BUTTON);
  Gtk::Box* button_box = MK_HBOX(5);
  button_box->pack_start(*MK_HSEP, true, true, 0);
  button_box->pack_start(*close_but, false, false, 0);

  Gtk::Box* main_vbox = MK_VBOX(5);
  main_vbox->set_border_width(5);
  main_vbox->pack_start(*info_frame, true, true, 0);
  main_vbox->pack_end(*button_box, false, false, 0);

  close_but->signal_clicked().connect(sigc::mem_fun(*this, &WinBase::close));

  this->add(*main_vbox);
  this->set_title("Skill Details");
  this->set_default_size(350, 300);
  this->show_all();
}

/* ---------------------------------------------------------------- */

void
GuiSkill::set_skill (int skill_id)
{
  this->skill_id.set_text("ID " + Helpers::get_string_from_int(skill_id));
  this->group_id.set_text("---");
  this->group_name.set_text("---");
  this->skill_attribs.set_text("---");

  ApiSkill const* skill = 0;
  ApiSkillGroup const* group = 0;
  try
  {
    ApiSkillTreePtr tree = ApiSkillTree::request();
    skill = tree->get_skill_for_id(skill_id);
    if (skill != 0)
      group = tree->get_group_for_id(skill->group);
  }
  catch (...)
  {
  }

  if (skill == 0)
  {
    this->skill_name.set_text("Unknown skill!");
    return;
  }

  this->skill_name.set_text(skill->name + " ("
      + Helpers::get_string_from_int(skill->rank) + ")");
  this->skill_attribs.set_text(ApiSkillTree::get_attrib_name(skill->primary)
      + std::string(" / ") + ApiSkillTree::get_attrib_name(skill->secondary));
  this->desc_buffer->set_text(skill->desc);

  int group_id = skill->group;
  this->group_id.set_text("ID " + Helpers::get_string_from_int(group_id));
  if (group == 0)
  {
    this->group_name.set_text("Unknown group!");
    return;
  }
  this->group_name.set_text(group->name);

  this->set_title("Skill Details: " + skill->name + " ("
      + Helpers::get_string_from_int(skill->rank) + ")");
}
