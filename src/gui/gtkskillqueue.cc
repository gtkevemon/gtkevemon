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
#include "api/evetime.h"
#include "api/apiskilltree.h"
#include "api/apiskillqueue.h"
#include "bits/config.h"
#include "imagestore.h"
#include "gtkhelpers.h"
#include "gtkdefines.h"
#include "gtkskillqueue.h"
#include "guiskill.h"

GtkSkillQueueViewCols::GtkSkillQueueViewCols (Gtk::TreeView* view,
    GtkSkillQueueColumns* cols)
  : GtkColumnsBase(view),
    position("Position", cols->queue_pos),
    start_sp("Start SP", cols->start_sp),
    end_sp("Dest SP", cols->end_sp),
    start_time("Start time", cols->start_time),
    end_time("Finish time", cols->end_time),
    duration("Duration", cols->duration),
    training("Training", cols->training),
    attribs("Attribs", cols->attribs)
{
  this->skill_name.set_title("Skill name");
  this->skill_name.pack_start(cols->skill_icon, false);
  this->skill_name.pack_start(cols->skill_name, true);

  this->position.set_resizable(false);

  this->append_column(&this->position,
      GtkColumnOptions(false, true, true, ImageStore::columnconf[1]));
  this->append_column(&this->skill_name, GtkColumnOptions(false, false, true));
  this->append_column(&this->start_sp, GtkColumnOptions(true, false, true));
  this->append_column(&this->end_sp, GtkColumnOptions(true, false, true));
  this->append_column(&this->start_time, GtkColumnOptions(true, false, true));
  this->append_column(&this->end_time, GtkColumnOptions(true, false, true));
  this->append_column(&this->duration, GtkColumnOptions(true, false, true));
  this->append_column(&this->training, GtkColumnOptions(true, false, true));
  this->append_column(&this->attribs, GtkColumnOptions(true, false, true));

  this->position.get_first_cell()->set_property("xalign", 1.0f);
  this->skill_name.set_expand(true);
  this->start_sp.get_first_cell()->set_property("xalign", 1.0f);
  this->end_sp.get_first_cell()->set_property("xalign", 1.0f);
}

/* ================================================================ */

GtkSkillQueue::GtkSkillQueue (void)
  : queue_store(Gtk::ListStore::create(queue_cols)),
    queue_view(queue_store),
    queue_view_cols(&queue_view, &queue_cols)
{
  /* Setup EVE API fetcher. */
  Gtk::ScrolledWindow* scwin = MK_SCWIN;
  scwin->add(this->queue_view);
  scwin->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_ALWAYS);
  scwin->set_shadow_type(Gtk::SHADOW_ETCHED_IN);

  this->queue_view_cols.position.signal_clicked().connect(sigc::mem_fun
      (this->queue_view_cols, &GtkColumnsBase::toggle_edit_context));

  this->set_border_width(5);
  this->pack_start(*scwin, true, true, 0);

  //this->queue_view_cols.set_format("+0 +1 +2 +3 +4 +5 +6 +7");
  this->init_from_config();
  this->queue_view_cols.setup_columns_normal();

  this->queue_view.signal_row_activated().connect(sigc::mem_fun
                (*this, &GtkSkillQueue::on_row_activated));
}

/* ---------------------------------------------------------------- */

GtkSkillQueue::~GtkSkillQueue (void)
{
  this->store_to_config();
  Config::save_to_file();
}

/* ---------------------------------------------------------------- */

void
GtkSkillQueue::set_character (CharacterPtr character)
{
  this->character = character;
  this->character->signal_skill_queue_updated().connect
      (sigc::mem_fun(*this, &GtkSkillQueue::on_apidata_available));
  this->character->signal_cached_warning().connect(sigc::bind
      (sigc::mem_fun(*this, &GtkSkillQueue::on_api_problems), true));
  this->character->signal_request_error().connect(sigc::bind
      (sigc::mem_fun(*this, &GtkSkillQueue::on_api_problems), false));
}

/* ---------------------------------------------------------------- */

void
GtkSkillQueue::refresh (void)
{
    if (this->character->valid_training_sheet())
        this->on_apidata_available();
    else
        this->character->request_skillqueue();
}

/* ---------------------------------------------------------------- */

void
GtkSkillQueue::init_from_config (void)
{
  ConfSectionPtr skillqueue = Config::conf.get_section("skillqueue");
  ConfValuePtr columns_format = skillqueue->get_value("columns_format");
  this->queue_view_cols.set_format(**columns_format);
}

/* ---------------------------------------------------------------- */

void
GtkSkillQueue::store_to_config (void)
{
  ConfSectionPtr skillqueue = Config::conf.get_section("skillqueue");
  ConfValuePtr columns_format = skillqueue->get_value("columns_format");
  columns_format->set(this->queue_view_cols.get_format());
}

/* ---------------------------------------------------------------- */

#include <iostream>

void
GtkSkillQueue::on_apidata_available (void)
{
  ApiSkillQueuePtr sq = this->character->sq;

  /* Debugging. */
  //sq->debug_dump();

  /* FIXME: Pay attention to training times. */

  this->queue_store->clear();
  ApiSkillTreePtr tree = ApiSkillTree::request();
  time_t training = 0;
  for (std::size_t i = 0; i < sq->queue.size(); ++i)
  {
    ApiSkillQueueItem const& item = sq->queue[i];

    time_t duration = item.end_time_t - item.start_time_t;
    //time_t duration = item.end_time_t - EveTime::get_eve_time(); //FIX?
    training += duration;

    ApiSkill const* skill = tree->get_skill_for_id(item.skill_id);
    std::string skill_name = (skill == 0
        ? Helpers::get_string_from_int(item.skill_id) : skill->name);
    skill_name += " ";
    skill_name += Helpers::get_roman_from_int(item.to_level);

    Gtk::TreeModel::Row row = *this->queue_store->append();
    row[this->queue_cols.queue_pos] = item.queue_pos;
    row[this->queue_cols.skill_name] = skill_name;
    row[this->queue_cols.skill_icon] = ImageStore::skillicons[1];
    row[this->queue_cols.start_sp] = item.start_sp;
    row[this->queue_cols.end_sp] = item.end_sp;
    row[this->queue_cols.start_time]
        = EveTime::get_local_time_string(item.start_time_t, true);
    row[this->queue_cols.end_time]
        = EveTime::get_local_time_string(item.end_time_t, true);
    row[this->queue_cols.duration]
        = EveTime::get_string_for_timediff(duration, true);
    row[this->queue_cols.training]
        = EveTime::get_string_for_timediff(training, true);

    Glib::ustring attribs;
    attribs += ApiSkillTree::get_attrib_short_name(skill->primary);
    attribs += " / ";
    attribs += ApiSkillTree::get_attrib_short_name(skill->secondary);
    row[this->queue_cols.attribs]
      = attribs;

    row[this->queue_cols.skill_id] = item.skill_id;
  }
}

/* ---------------------------------------------------------------- */

void
GtkSkillQueue::on_api_problems (EveApiDocType sheet,
    std::string error, bool cache)
{
  if (sheet != API_DOCTYPE_SKILLQUEUE)
    return;
  this->raise_error(error, cache);
}

/* ---------------------------------------------------------------- */

void
GtkSkillQueue::raise_error (std::string const& error, bool cached)
{
  Gtk::MessageType message_type;
  Glib::ustring message;
  if (cached)
  {
    message = "Using cached version of the skill queue!";
    message_type = Gtk::MESSAGE_WARNING;
  }
  else
  {
    message = "Error retrieving skill queue!";
    message_type = Gtk::MESSAGE_ERROR;
  }

  Gtk::Window* win = (Gtk::Window*)this->get_toplevel();
  Gtk::MessageDialog md(*win, message, false, message_type, Gtk::BUTTONS_OK);
  md.set_secondary_text("There was an error while requesting the skill "
      "queue from the EVE API. The EVE API is either offline, or the "
      "requested document is not understood by GtkEveMon. "
      "The error message is:\n\n" + GtkHelpers::locale_to_utf8(error));
  md.set_title("Error - GtkEveMon");
  md.run();
}

void GtkSkillQueue::on_row_activated(Gtk::TreeModel::Path const& path,
    Gtk::TreeViewColumn* /*col*/)
{
  Gtk::TreeModel::iterator iter = this->queue_store->get_iter(path);
  Gtk::TreeModel::Row row = *iter;
  int skill_id = row[this->queue_cols.skill_id];
  if (skill_id >= 0)
  {
    GuiSkill* skillgui = new GuiSkill();
    skillgui->set_skill(skill_id);
  }
}
