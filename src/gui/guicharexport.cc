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
#include <iomanip>
#include <sstream>

#include <gtkmm.h>

#include "util/helpers.h"
#include "gtkdefines.h"
#include "guicharexport.h"

GtkExportPane::GtkExportPane (void)
    : text_buf(Gtk::TextBuffer::create())
{
    Gtk::TextView* text_view = Gtk::manage(new Gtk::TextView(this->text_buf));
    text_view->set_editable(false);

    Gtk::ScrolledWindow* scwin = MK_SCWIN;
    scwin->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_ALWAYS);
    scwin->set_shadow_type(Gtk::SHADOW_ETCHED_IN);
    scwin->add(*text_view);

    this->pack_start(*scwin, true, true, 0);
    this->set_border_width(5);
}

/* ---------------------------------------------------------------- */

void
GtkExportPane::set_text (Glib::ustring const& text)
{
    this->text_buf->insert(this->text_buf->end(), text);
    this->text_buf->select_range
        (this->text_buf->begin(), this->text_buf->begin());
}

/* ---------------------------------------------------------------- */

GuiCharExport::GuiCharExport (ApiCharSheetPtr charsheet)
{
  Gtk::Button* close_but = MK_BUT0;
  close_but->set_image_from_icon_name("widnow-close", Gtk::ICON_SIZE_BUTTON);
  Gtk::Box* button_box = MK_HBOX(5);
  button_box->pack_start(*MK_HSEP, true, true, 0);
  button_box->pack_start(*close_but, false, false, 0);

  GtkExportPane* bbc_export = Gtk::manage(new GtkExportPane());
  GtkExportPane* eft_export = Gtk::manage(new GtkExportPane());

  Gtk::Notebook* notebook = MK_NOTEBOOK;
  notebook->append_page(*eft_export, "EFT", false);
  notebook->append_page(*bbc_export, "BB-Code", false);

  Gtk::Box* main_box = MK_VBOX(5);
  main_box->set_border_width(5);
  main_box->pack_start(*notebook, true, true, 0);
  main_box->pack_start(*button_box, false, false, 0);

  close_but->signal_clicked().connect(sigc::mem_fun(*this, &WinBase::close));

  this->add(*main_box);
  this->set_title("Character Exports - GtkEveMon");
  this->set_default_size(500, 600);
  this->show_all();

  this->fill_eft(charsheet, eft_export);
  this->fill_bbc(charsheet, bbc_export);
}

/* ---------------------------------------------------------------- */

void
GuiCharExport::fill_eft (ApiCharSheetPtr charsheet, GtkExportPane* pane)
{
    std::stringstream ss;
    for (std::size_t i = 0; i < charsheet->skills.size(); ++i)
    {
        ApiCharSheetSkill const& skill(charsheet->skills[i]);
        ss << skill.details->name << "=" << skill.level << std::endl;
    }
    pane->set_text(ss.str());
}

/* ---------------------------------------------------------------- */

void
GuiCharExport::fill_bbc (ApiCharSheetPtr charsheet, GtkExportPane* pane)
{
    std::stringstream ss;
    ss << "[b]" << charsheet->name << "[/b]\n\n";
    ss << "[b]Attributes[/b]\n";
    ss << "Intelligence: " << charsheet->total.intl << "\n";
    ss << "Perception: " << charsheet->total.per << "\n";
    ss << "Charisma: " << charsheet->total.cha << "\n";
    ss << "Willpower: " << charsheet->total.wil << "\n";
    ss << "Memory: " << charsheet->total.mem << "\n\n";

    /* Create the data structure. */
    GroupMap data;
    GuiCharExport::create_datamap(data, charsheet);

    /* Make a list with amount of skills per level. */
    std::vector<unsigned int> skills_at;
    skills_at.resize(6, 0);

    /* Iterate over all skill groups. */
    unsigned int grandtotal = 0;
    for (GroupMap::iterator iter = data.begin(); iter != data.end(); iter++)
    {
        ss << "[b]" << iter->first << "[/b]\n";

        /* Iterate over skills in this skill group. */
        unsigned int total = 0;
        for (SkillMap::iterator siter = iter->second.begin();
            siter != iter->second.end(); siter++)
        {
            ss << "[img]http://myeve.eve-online.com/bitmaps/character/level"
               << siter->second->level << ".gif[/img] "
               << siter->first << "\n";

            /* Count total skill points for the group. */
            total += siter->second->points;

            /* Count skill for the skill level. */
            skills_at[siter->second->level] += 1;
        }

        grandtotal += total;
        ss << "Total Skillpoints in Group: "
           << Helpers::get_dotted_str_from_uint(total) << "\n\n";
    }

    /* Misc info */
    ss << "Total Skillpoints: "
       << Helpers::get_dotted_str_from_uint(grandtotal) << "\n";
    ss << "Total Number of Skills: "
       << Helpers::get_string_from_sizet(charsheet->skills.size()) << "\n\n";

    for (unsigned int i = 0; i < skills_at.size(); ++i)
        ss << "Skills at Level " << i << ": " << skills_at[i] << "\n";

    pane->set_text(ss.str());
}

/* ---------------------------------------------------------------- */

void
GuiCharExport::create_datamap (GroupMap& result, ApiCharSheetPtr charsheet)
{
  ApiSkillTreePtr tree = ApiSkillTree::request();

  for (unsigned int i = 0; i < charsheet->skills.size(); ++i)
  {
    ApiSkill const* skill = charsheet->skills[i].details;
    ApiSkillGroup const* group = tree->get_group_for_id(skill->group);
    if (group == 0)
    {
      std::cout << "Warning: Group " << skill->group
          << " does not exist!" << std::endl;
      continue;
    }

    std::string groupname = group->name;
    result[groupname][skill->name] = &charsheet->skills[i];
  }
}
