#include <iostream>

#include "util/helpers.h"
#include "api/evetime.h"

#include "character.h"

Character::Character (EveApiAuth const& auth)
{
  this->auth = auth;

  this->cs_fetcher.set_auth(auth);
  this->sq_fetcher.set_auth(auth);

  this->cs_fetcher.set_doctype(API_DOCTYPE_CHARSHEET);
  this->sq_fetcher.set_doctype(API_DOCTYPE_SKILLQUEUE);

  this->cs = ApiCharSheet::create();
  this->sq = ApiSkillQueue::create();

  this->cs_fetcher.signal_done().connect(sigc::mem_fun
      (*this, &Character::on_cs_available));
  this->sq_fetcher.signal_done().connect(sigc::mem_fun
      (*this, &Character::on_sq_available));

  this->process_api_data();
}

/* ---------------------------------------------------------------- */

Character::~Character (void)
{
  //std::cout << "Removing character" << std::endl;
}

/* ---------------------------------------------------------------- */

void
Character::on_cs_available (EveApiData data)
{
  if (data.data.get() == 0)
  {
    this->sig_request_error.emit(API_DOCTYPE_CHARSHEET, data.exception);
    return;
  }

  bool yet_unnamed = this->cs->name.empty();

  try
  {
    this->cs->set_api_data(data);
    if (data.locally_cached)
      this->sig_cached_warning.emit(API_DOCTYPE_CHARSHEET, data.exception);
  }
  catch (Exception& e)
  {
    this->sig_request_error.emit(API_DOCTYPE_CHARSHEET, e);
    return;
  }

  if (yet_unnamed && !this->cs->name.empty())
    this->sig_name_available.emit(this->auth.char_id);

  this->process_api_data();
  this->sig_char_sheet_updated.emit();
  this->sig_api_info_changed.emit();
}

/* ---------------------------------------------------------------- */

void
Character::on_sq_available (EveApiData data)
{
  if (data.data.get() == 0)
  {
    this->sig_request_error.emit(API_DOCTYPE_SKILLQUEUE, data.exception);
    return;
  }

  try
  {
    this->sq->set_api_data(data);
    if (data.locally_cached)
      this->sig_cached_warning.emit(API_DOCTYPE_SKILLQUEUE, data.exception);
  }
  catch (Exception& e)
  {
    this->sig_request_error.emit(API_DOCTYPE_SKILLQUEUE, e);
    return;
  }

  this->process_api_data();
  this->sig_skill_queue_updated.emit();
  this->sig_api_info_changed.emit();
}

/* ---------------------------------------------------------------- */

void
Character::process_api_data (void)
{
    this->training_skill = 0;
    this->training_level_sp = 0;
    this->training_skill_sp = 0;
    this->training_level_done = 0.0;
    this->training_remaining = 0;
    this->training_info.queue_pos = -1;

    this->char_base_sp = 0;
    this->char_live_sp = 0;
    this->char_group_base_sp = 0;

    /* Update information related to the character sheet. */
    if (this->cs->valid)
    {
        this->char_base_sp = this->cs->total_sp;
        this->char_live_sp = this->cs->total_sp;
    }

    /* Update information related to the skill queue. */
    if (this->is_training())
    {
        ApiSkillTreePtr tree = ApiSkillTree::request();
        this->training_info = *this->sq->get_training_skill();
        this->training_skill = tree->get_skill_for_id(this->training_info.skill_id);

        if (this->training_skill == 0)
        {
            std::cout << "Warning: Skill in training (ID "
                << this->training_info.skill_id << ") not found. "
                << "Skill tree out of date?" << std::endl;
        }
    }

    /* Update information related to both sheets. */
    if (this->cs->valid && this->sq->valid)
    {
        this->training_cskill = 0;
        if (this->is_training())
        {
            /* Get the character skill in training. */
            this->training_cskill = this->cs->get_skill_for_id
                (this->training_info.skill_id);

            if (this->training_cskill != 0)
            {
                /* Cache the amount of SP in the active skill group. */
                int group_id = this->training_cskill->details->group;
                for (std::size_t i = 0; i < this->cs->skills.size(); ++i)
                {
                    ApiCharSheetSkill& cskill = this->cs->skills[i];
                    if (cskill.details->group == group_id)
                        this->char_group_base_sp += cskill.points;
                }
            }
            else
            {
                std::cout << "Warning: Skill in training (ID "
                    << this->training_info.skill_id
                    << ") is unknown to " << this->cs->name
                    << "!" << std::endl;
            }
        }

        /* Update the character from skill queue information. */
        this->update_from_queue();
    }

    /* Update the live info. */
    this->update_live_info();
}

/* ---------------------------------------------------------------- */

void
Character::update_live_info (void)
{
  if (this->training_info.queue_pos < 0)
    return;

  time_t evetime = EveTime::get_eve_time();
  time_t finish = this->training_info.end_time_t;
  time_t diff = finish - evetime;

  //std::cout << "** Evetime: " << evetime << ", Finish: " << finish
  //    << ", time diff: " << diff << std::endl;

  /* Check if the skill is finished. */
  if (diff < 0)
  {
    this->training_info.queue_pos = -1;
    this->skill_completed();
    return;
  }

  /* Update easy values first to get useful results even in case of errors. */
  unsigned int level_dest_sp = this->training_info.end_sp;
  double spps = 0.0;
  /* The easy values aren't as easy as one would hope, though. Compute
   * the spph of the current skill if we can, otherwise call it 0. */
  if (this->cs->valid && this->training_skill)
  {
    spps = this->cs->get_spph_for_skill(this->training_skill) / 3600.0;
  }

  this->training_remaining = diff;
  this->training_skill_sp = level_dest_sp - (unsigned int)((double)diff * spps);

  /* Check if the skill in training could be determined.
   * This may be NULL if the skill was not available in the skill tree. */
  if (this->training_skill == 0)
    return;

  /* Update training live values. */
  unsigned int level_start_sp = ApiCharSheet::calc_start_sp
      (this->training_info.to_level - 1, this->training_skill->rank);
  unsigned int level_total_sp = level_dest_sp - level_start_sp;

  this->training_level_sp = this->training_skill_sp - level_start_sp;
  this->training_level_done = (double)this->training_level_sp
      / (double)level_total_sp;

  //std::cout << "** Start SP: " << level_start_sp << ", Dest SP: "
  //    << level_dest_sp << ", Level SP: " << level_total_sp << ", SP/s: "
  //    << spps << std::endl;

  /* Check if the skill in training is available in the character.
   * This may be NULL if the skill was not available in the skill tree. */
  if (!this->cs->valid || this->training_cskill == 0)
    return;

  /* Update character live values. */
  unsigned int basediff_sp = this->training_skill_sp
      - this->training_cskill->points;
  this->char_live_sp = this->char_base_sp + basediff_sp;
  this->char_group_live_sp = this->char_group_base_sp + basediff_sp;
}

/* ---------------------------------------------------------------- */

void
Character::update_from_queue (void)
{
    if (!this->sq->valid || !this->cs->valid)
        return;

    /* Walk through the skill queue and update the character with already
     * finished skills and the previous level of the skill in training.
     */
    time_t eve_time = EveTime::get_eve_time();
    for (std::size_t i = 0; i < this->sq->queue.size(); ++i)
    {
        ApiSkillQueueItem const& item(this->sq->queue[i]);
        if (item.start_time_t > eve_time)
            break;
        if (item.end_time_t < eve_time)
            this->cs->add_char_skill(item.skill_id, item.to_level);
        else
            this->cs->add_char_skill(item.skill_id, item.to_level - 1);
    }
    this->char_base_sp = this->cs->total_sp;
}

/* ---------------------------------------------------------------- */

void
Character::skill_completed (void)
{
    /* Update the character by registering completed skills. */
    this->update_from_queue();

    /* Emit the completed signal to notify listeners. */
    this->sig_skill_completed.emit();

    /* Clear old skill info and reprocess API data to update. */
    this->training_cskill = 0;
    this->training_skill = 0;
    this->process_api_data();

    /* Emit signal to notify about changed skill in training. */
    this->sig_training_changed.emit();
}

/* ---------------------------------------------------------------- */

std::string
Character::get_char_name (void) const
{
  if (this->cs->valid)
    return this->cs->name;
  else
    return this->auth.char_id;
}

/* ---------------------------------------------------------------- */

std::string
Character::get_training_text (void) const
{
    if (this->is_training())
    {
        int skill_id = this->training_info.skill_id;
        int to_level = this->training_info.to_level;

        std::string to_level_str = Helpers::get_roman_from_int(to_level);
        std::string skill_str;
        try
        {
            ApiSkillTreePtr tree = ApiSkillTree::request();
            ApiSkill const* skill = tree->get_skill_for_id(skill_id);
            if (skill == 0)
                throw Exception();
            skill_str = skill->name;
        }
        catch (Exception& e)
        {
            /* This happens if the ID is not found. */
            skill_str = Helpers::get_string_from_int(skill_id);
        }

        return skill_str + " " + to_level_str;
    }
    else
    {
        return "No skill in training!";
    }
}

/* ---------------------------------------------------------------- */

std::string
Character::get_remaining_text (bool slim) const
{
    if (!this->sq->valid)
        return "No training information!";

    if (this->sq->is_paused())
        return "Training is paused!";

    if (!this->is_training())
        return "No skill in training!";

    return EveTime::get_string_for_timediff(this->training_remaining, slim);
}

/* ---------------------------------------------------------------- */

std::string
Character::get_summary_text (bool detailed)
{
    std::string ret;
    ret += this->get_char_name();
    ret += " - ";

    if (this->is_training())
    {
        ret += this->get_remaining_text(true);
        if (detailed)
        {
            ret += " - ";
            ret += this->get_training_text();
        }
    }
    else
        ret += "Not training!";

    return ret;
}
