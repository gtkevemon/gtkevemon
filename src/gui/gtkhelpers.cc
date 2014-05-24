#include <string>
#include <sstream>
#include <gtkmm/image.h>

#include "util/helpers.h"
#include "api/evetime.h"
#include "imagestore.h"
#include "gtkhelpers.h"

void
GtkHelpers::create_tooltip (Glib::RefPtr<Gtk::Tooltip> const& tooltip,
        ApiSkill const* skill, ApiCharSheetSkill* cskill,
        CharacterPtr character)
{
  tooltip->set_icon(ImageStore::skill);

  std::stringstream ss;

  if (cskill != 0)
    ss << "Name: " << skill->name << " "
        << Helpers::get_roman_from_int(cskill->level) << "\n";
  else
    ss << "Name: " << skill->name << "\n";

  ss << "Attributes: " << ApiSkillTree::get_attrib_name(skill->primary)
     << " / " << ApiSkillTree::get_attrib_name(skill->secondary)
     << "\nRank: " << skill->rank << "\n";

  if (cskill != 0 && cskill->level != 5)
  {
    /* Fill basic information available from the character skill. */
    int current_sp = cskill->points;
    double completed = cskill->completed;

    /* Get more information available from char/training sheet. */
    double spph = 0.0;
    time_t time_remaining = 0;

    if (character.get() != 0 && character->cs->valid)
    {
      /* Insert live information if the current skill is in training. */
      if (character->is_training() && character->training_skill == skill)
      {
        /* Fill live information. */
        current_sp = character->training_skill_sp;
        completed = character->training_level_done;
      }

      /* Fill information from char sheet. */
      int sp_remaining = cskill->points_dest - current_sp;
      spph = (double)character->cs->get_spph_for_skill(skill);
      time_remaining = (time_t)(3600.0 * (double)sp_remaining / spph);
    }

    ss << "Skill level from " << Helpers::get_dotted_str_from_int
        (cskill->points_start) << " to " << Helpers::get_dotted_str_from_int
        (cskill->points_dest) << " SP\n";

    if (cskill->points_start != current_sp)
      ss << "Current SP: " << Helpers::get_dotted_str_from_int
          (current_sp) << "\n";

    if (spph != 0.0 && time_remaining != 0)
    {
      ss << "SP per hour: " << (int)spph << "\n";
      if (completed != 0.0)
        ss << "Remaining time ";
      else
        ss << "Training time ";
      ss << "to level " << Helpers::get_roman_from_int(cskill->level + 1) << ": ";
      ss << EveTime::get_string_for_timediff(time_remaining, false) << "\n";
    }

    for (int level = cskill->level + 2; level <= 5; level++)
    {
      time_remaining = (time_t)(3600.0
        * (double)(ApiCharSheet::calc_dest_sp(
        level - 1, cskill->details->rank)
        - ApiCharSheet::calc_start_sp(level - 1,
        cskill->details->rank)) / spph);
      ss << "Training time to level "
        << Helpers::get_roman_from_int(level) << ": "
        << EveTime::get_string_for_timediff(time_remaining, false) << "\n";
    }

    if (completed != 0.0)
      ss << "Completed: " << Helpers::get_string_from_double
          (completed * 100.0, 2) << "%\n";
  }


#if 0
  /* Old version without live SP counting. */
  if (cskill != 0 && sheet.get() != 0 && sheet->valid && cskill->level != 5)
  {
    int sp_to_go = cskill->points_dest - cskill->points;
    double spph = (double)sheet->get_spph_for_skill(skill);
    time_t secs_next_level = (time_t)(3600.0 * (double)sp_to_go / spph);
    std::string next_level_str = EveTime::get_string_for_timediff
        (secs_next_level, false);
    if (cskill->completed != 0.0)
      next_level_str += " (" + Helpers::get_string_from_double
          (cskill->completed * 100.0, 0) + "% completed)";
    ss << "SP per hour: " << (int)(spph) << "\n"
        << "Destination SP: "
        << Helpers::get_dotted_str_from_int(cskill->points_dest) << "\n"
        << "Training time: " << next_level_str << "\n";
  }
#endif

  ss << "\n" << skill->desc;
  tooltip->set_text(ss.str());
}

/* ---------------------------------------------------------------- */

void
GtkHelpers::create_tooltip (Glib::RefPtr<Gtk::Tooltip> const& tooltip,
    ApiCert const* cert)
{
  Glib::ustring class_name = cert->class_details->name;
  Glib::ustring cert_grade = ApiCertTree::get_name_for_grade(cert->grade);
  Glib::ustring cert_desc = cert->desc;

  tooltip->set_icon(ImageStore::certificate);
  tooltip->set_text("Name: " + class_name + "\nGrade: " + cert_grade
      + "\n\n" + cert_desc);
}

/* ---------------------------------------------------------------- */

bool
GtkHelpers::create_tooltip_from_view (int x, int y,
    Glib::RefPtr<Gtk::Tooltip> tip, Gtk::TreeView& view,
    Glib::RefPtr<Gtk::TreeModel> store,
    Gtk::TreeModelColumn<ApiElement const*> col)
{
  Gtk::TreeModel::Path path;
  Gtk::TreeViewDropPosition pos;
  bool exists = view.get_dest_row_at_pos(x, y, path, pos);
  if (!exists)
    return false;

  Gtk::TreeIter iter = store->get_iter(path);
  ApiElement const* elem = (*iter)[col];
  if (elem == 0)
    return false;

  /* Reposition tooltip. */
  view.set_tooltip_row(tip, path);

  switch (elem->get_type())
  {
    case API_ELEM_SKILL:
      GtkHelpers::create_tooltip(tip, (ApiSkill const*)elem);
      break;
    case API_ELEM_CERT:
      GtkHelpers::create_tooltip(tip, (ApiCert const*)elem);
      break;
    default:
      return false;
  }

  return true;
}

/* ---------------------------------------------------------------- */

std::string
GtkHelpers::locale_to_utf8 (Glib::ustring const& opsys_string)
{
  /* We don't throw the error further away, we'll handle it here. */
  #define LOCALE_TO_UTF8_ERROR "Error converting string to UTF-8!"
  #ifdef GLIBMM_EXCEPTIONS_ENABLED
  try
  { return Glib::locale_to_utf8(opsys_string); }
  catch (...)
  { return LOCALE_TO_UTF8_ERROR; }
  #else
  std::auto_ptr<Glib::Error> error;
  std::string ret = Glib::locale_to_utf8(opsys_string, error);
  if (error.get())
    return LOCALE_TO_UTF8_ERROR;
  return ret;
  #endif
}
