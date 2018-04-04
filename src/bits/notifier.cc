#include <iostream>

#include "api/evetime.h"
#include "api/apiskilltree.h"
#include "util/exception.h"
#include "util/pipedexec.h"
#include "util/helpers.h"

#include "config.h"
#include "notifier.h"

int
Notifier::exec (CharacterPtr character)
{
  ApiCharSheetPtr cs = character->cs;

  /* Check if the sheets are valid. */
  if (!cs->valid)
    throw Exception("Character sheet is invalid. Please report this issue!");

  /* Receive configuration values. */
  std::string command = **Config::conf.get_value("notifications.exec_command");
  std::string data = **Config::conf.get_value("notifications.exec_data");
  std::string minspstr = **Config::conf.get_value("notifications.minimum_sp");

  if (command.empty())
    throw Exception("Bad command specified, check the configuration.");

  /* Convert minimum SP string to int. */
  int minsp = 0;
  if (!minspstr.empty())
    minsp = Helpers::get_int_from_string(minspstr);

  /* Collect some required information. */
  int training_id = character->training_info.skill_id;
  int to_level = character->training_info.to_level;
  ApiSkillTreePtr tree = ApiSkillTree::request();
  ApiSkill const* skill = tree->get_skill_for_id(training_id);

  if (skill == 0)
    throw Exception("The skill in training is invalid. Update data files?");

  /* Prepare some more information. */
  int start_sp = ApiCharSheet::calc_start_sp(to_level - 1, skill->rank);
  int dest_sp = ApiCharSheet::calc_dest_sp(to_level - 1, skill->rank);
  int diff_sp = dest_sp - start_sp;

  /* Simply return if there is no need to execute the handler. */
  if (diff_sp < minsp)
  {
    std::cout << "Minimum SP: " << minsp << ", skill SP: " << diff_sp
        << ", NOT executing handler!" << std::endl;
    return 0;
  }
  else
  {
    std::cout << "Minimum SP: " << minsp << ", skill SP: " << diff_sp
        << ", executing handler!" << std::endl;
  }

  /* Prepare even more information. */
  /* This spph value isn't actually the average SP/hr over the skill train,
   * but I'm pretty sure there is no way to actually get that given
   * the bogus values the API returns. */
  unsigned int spph = character->cs->get_spph_for_skill(skill);
  double spps = (double)spph / 3600.0;

  std::string to_level_str = Helpers::get_roman_from_int(to_level);
  std::string start_sp_str = Helpers::get_dotted_str_from_int(start_sp);
  std::string dest_sp_str = Helpers::get_dotted_str_from_int(dest_sp);
  std::string new_sp_str = Helpers::get_dotted_str_from_int(diff_sp);
  std::string spph_str = Helpers::get_string_from_int(spph);
  std::string train_time = EveTime::get_string_for_timediff
      ((time_t)(diff_sp / spps), false);
  std::string eve_time = EveTime::get_eve_time_string();
  std::string local_time = EveTime::get_local_time_string();

  /* Parse command and data and substitude variables. */
  Notifier::replace(command, "$CHAR", cs->name);
  Notifier::replace(command, "$SKILL", skill->name);
  Notifier::replace(command, "$LEVEL", to_level_str);
  Notifier::replace(command, "$STARTSP", start_sp_str);
  Notifier::replace(command, "$DESTSP", dest_sp_str);
  Notifier::replace(command, "$NEWSP", new_sp_str);
  Notifier::replace(command, "$SPPH", spph_str);
  Notifier::replace(command, "$TRAINTIME", train_time);
  Notifier::replace(command, "$EVETIME", eve_time);
  Notifier::replace(command, "$LOCALTIME", local_time);

  Notifier::replace(data, "$CHAR", cs->name);
  Notifier::replace(data, "$SKILL", skill->name);
  Notifier::replace(data, "$LEVEL", to_level_str);
  Notifier::replace(data, "$STARTSP", start_sp_str);
  Notifier::replace(data, "$DESTSP", dest_sp_str);
  Notifier::replace(data, "$NEWSP", new_sp_str);
  Notifier::replace(data, "$SPPH", spph_str);
  Notifier::replace(data, "$TRAINTIME", train_time);
  Notifier::replace(data, "$EVETIME", eve_time);
  Notifier::replace(data, "$LOCALTIME", local_time);
  Notifier::replace(data, "$NL", "\n");

  /* Execute handler. */
  StringVector argv = Helpers::tokenize_cmd(command);

  PipedExec pe;
  pe.exec(argv);

  if (!data.empty())
    pe.send_data(data);
  pe.close_sender();

  return pe.waitpid();
}

/* ---------------------------------------------------------------- */

void
Notifier::replace (std::string& subject, std::string const& find,
    std::string const& replace)
{
  while (true)
  {
    size_t pos = subject.find(find);
    if (pos == std::string::npos)
      break;
    subject.replace(pos, find.size(), replace);
  }
}
