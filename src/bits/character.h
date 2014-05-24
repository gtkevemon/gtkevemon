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

#ifndef CHARACTER_HEADER
#define CHARACTER_HEADER

#include <string>
#include <sigc++/signal.h>

#include "util/ref_ptr.h"
#include "api/eveapi.h"
#include "api/apicharsheet.h"
#include "api/apiskillqueue.h"

/* TODO
 * Emit signal if clone runs out of SP?
 * Move caching to this class? This enables to read API errors
 */

class Character;
typedef ref_ptr<Character> CharacterPtr;

class Character
{
  public:
    typedef sigc::signal<void, EveApiDocType, std::string> SignalRequestError;
    typedef sigc::signal<void, EveApiDocType, std::string> SignalCachedWarning;
    typedef sigc::signal<void, std::string> SignalNameAvailable;
    typedef sigc::signal<void> SignalApiInfoChanged;
    typedef sigc::signal<void> SignalCharSheetUpdated;
    typedef sigc::signal<void> SignalSkillQueueUpdated;
    typedef sigc::signal<void> SignalSkillCompleted;
    typedef sigc::signal<void> SignalTrainingChanged;

  protected:
    EveApiAuth auth;
    EveApiFetcher cs_fetcher;
    EveApiFetcher sq_fetcher;

    SignalRequestError sig_request_error;
    SignalCachedWarning sig_cached_warning;
    SignalNameAvailable sig_name_available;
    SignalApiInfoChanged sig_api_info_changed;
    SignalCharSheetUpdated sig_char_sheet_updated;
    SignalSkillQueueUpdated sig_skill_queue_updated;
    SignalSkillCompleted sig_skill_completed;
    SignalTrainingChanged sig_training_changed;

  public:
    /* API sheets. The sheets do not contain any live information. */
    ApiCharSheetPtr cs;
    ApiSkillQueuePtr sq;

    /* Information if the charsheet is available. */
    unsigned int char_base_sp;
    unsigned int char_live_sp; /* Live SP. */

    /* Information if the training sheet is available. */
    ApiSkill const* training_skill;
    ApiSkillQueueItem training_info;
    unsigned int training_level_sp; /* Live SP. */
    unsigned int training_skill_sp; /* Live SP. */
    double training_level_done; /* Live SP. */
    time_t training_remaining; /* Live SP. */

    /* Information if both, charsheet AND training are available. */
    ApiCharSheetSkill* training_cskill;
    unsigned int char_group_base_sp;
    unsigned int char_group_live_sp; /* Live SP. */

  protected:
    Character (EveApiAuth const& auth);

    void on_cs_available (EveApiData data);
    void on_sq_available (EveApiData data);

    void process_api_data (void);
    void skill_completed (void);

  public:
    ~Character (void);

    /* Creates a character manager, needs API authentication. */
    static CharacterPtr create (EveApiAuth const& auth);

    /* API requests. Callers should obey the cache timers. */
    void request_charsheet (void);
    void request_skillqueue (void);

    /* Updates the live information, typically called every second. */
    void update_live_info (void);
    /* Updates the character with completed skills from the queue. */
    void update_from_queue (void);

    /* Getters and worked-up information. */
    std::string get_char_name (void) const;
    std::string const& get_user_id (void) const;
    std::string const& get_char_id (void) const;
    std::string get_training_text (void) const;
    std::string get_remaining_text (bool slim = false) const;
    std::string get_summary_text (bool detailed);
    bool is_training (void) const;

    bool valid_training_sheet (void);
    bool valid_character_sheet (void);

    /* Signals. */
    SignalRequestError& signal_request_error (void);
    SignalCachedWarning& signal_cached_warning (void);
    SignalNameAvailable& signal_name_available (void);
    SignalApiInfoChanged& signal_api_info_changed (void);
    SignalCharSheetUpdated& signal_char_sheet_updated (void);
    SignalSkillQueueUpdated& signal_skill_queue_updated (void);
    SignalSkillCompleted& signal_skill_completed (void);
    SignalTrainingChanged& signal_training_changed (void);
};

/* ---------------------------------------------------------------- */

inline CharacterPtr
Character::create (EveApiAuth const& auth)
{
  return CharacterPtr(new Character(auth));
}

inline std::string const&
Character::get_user_id (void) const
{
  return this->auth.user_id;
}

inline std::string const&
Character::get_char_id (void) const
{
  return this->auth.char_id;
}

inline void
Character::request_charsheet (void)
{
  if (!this->cs_fetcher.is_busy())
    this->cs_fetcher.async_request();
}

inline void
Character::request_skillqueue (void)
{
  if (!this->sq_fetcher.is_busy())
    this->sq_fetcher.async_request();
}

inline bool
Character::is_training (void) const
{
  return this->sq->valid && this->sq->in_training();
}

inline bool
Character::valid_character_sheet (void)
{
    return this->cs.get() && this->cs->valid;
}

inline bool
Character::valid_training_sheet (void)
{
    return this->sq.get() && this->sq->valid;
}

inline Character::SignalRequestError&
Character::signal_request_error (void)
{
  return this->sig_request_error;
}

inline Character::SignalCachedWarning&
Character::signal_cached_warning (void)
{
  return this->sig_cached_warning;
}

inline Character::SignalNameAvailable&
Character::signal_name_available (void)
{
  return this->sig_name_available;
}

inline Character::SignalApiInfoChanged&
Character::signal_api_info_changed (void)
{
  return this->sig_api_info_changed;
}

inline Character::SignalCharSheetUpdated&
Character::signal_char_sheet_updated (void)
{
  return this->sig_char_sheet_updated;
}

inline Character::SignalSkillQueueUpdated&
Character::signal_skill_queue_updated (void)
{
  return this->sig_skill_queue_updated;
}

inline Character::SignalSkillCompleted&
Character::signal_skill_completed (void)
{
  return this->sig_skill_completed;
}

inline Character::SignalTrainingChanged&
Character::signal_training_changed (void)
{
    return this->sig_training_changed;
}

#endif /* CHARACTER_HEADER */
