#include <gtkmm/stock.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/separator.h>
#include <gtkmm/table.h>

#include "util/helpers.h"
#include "api/evetime.h"
#include "guiplanattribopt.h"
#include "gtkdefines.h"
#include "gtktrainingplan.h"
#include "imagestore.h"

GtkTreeModelColumnsOptimizer::GtkTreeModelColumnsOptimizer (void)
{
  this->add(this->difference);
}

/* ---------------------------------------------------------------- */

GtkTreeViewColumnsOptimizer::GtkTreeViewColumnsOptimizer(Gtk::TreeView* view,
    GtkTreeModelColumnsOptimizer* cols)
  : GtkTreeViewColumns(view, cols),
    difference("Difference", cols->difference)
{
  this->append_column(&this->difference, GtkColumnOptions(true, true, true));
}

/* ---------------------------------------------------------------- */

GuiPlanAttribOpt::GuiPlanAttribOpt (void)
  : liststore(Gtk::ListStore::create(cols)),
    treeview(liststore),
    viewcols(&treeview, &cols)
{
  this->plan_offset = 0;

  Gtk::Widget* config_page = this->create_config_page();
  Gtk::Widget* attrib_page = this->create_attrib_page();
  Gtk::Widget* breakdown_page = this->create_breakdown_page();

  this->notebook.append_page(*config_page, "Configuration");
  this->notebook.append_page(*attrib_page, "New attributes");
  this->notebook.append_page(*breakdown_page, "Plan breakdown");

  /* Button bar. */
  Gtk::Button* close_but = MK_BUT(Gtk::Stock::CLOSE);
  Gtk::HBox* button_box = MK_HBOX;
  button_box->pack_start(*MK_HSEP, true, true, 0);
  button_box->pack_end(*close_but, false, false, 0);

  /* Main box. */
  Gtk::VBox* mainbox = MK_VBOX;
  mainbox->pack_start(this->notebook, true, true, 0);
  mainbox->pack_end(*button_box, false, false, 0);
  mainbox->set_border_width(5);

  /* Signals. */
  close_but->signal_clicked().connect(sigc::mem_fun(*this, &WinBase::close));

  /* Add the mainbox to the window, configure and start the dialog. */
  this->set_title("Attribute Optimizer - GtkEveMon");
  this->set_default_size(500, 500);
  this->add(*mainbox);
  this->show_all();
}

/* ---------------------------------------------------------------- */

Gtk::Widget*
GuiPlanAttribOpt::create_config_page (void)
{
  /* Some information on usage of the configuration. */
  Gtk::Image* info_image = MK_IMG(Gtk::Stock::DIALOG_QUESTION,
      Gtk::ICON_SIZE_DIALOG);
  Gtk::Label* info_label = MK_LABEL("Do you want to optimize the whole plan?\n"
      "Please select whether you want to optimize\n"
      "the whole skill plan or just a part of it.");
  info_label->set_line_wrap(true);
  info_label->set_justify(Gtk::JUSTIFY_LEFT);
  info_label->set_alignment(Gtk::ALIGN_LEFT);
  //info_label->set_width_chars(50);

  Gtk::HBox* info_box = MK_HBOX;
  info_box->set_spacing(10);
  info_box->pack_start(*info_image, false, false, 0);
  info_box->pack_start(*info_label, true, true, 0);

  /* Create the config widgets. */
  this->rb_whole_plan.set_label("Optimize the whole plan");
  this->rb_partial_plan.set_label("Optimize plan starting with skill");
  Gtk::RadioButtonGroup rbg;
  this->rb_whole_plan.set_group(rbg);
  this->rb_partial_plan.set_group(rbg);
  this->set_selection_sensitivity(false);

  /* Create a table and add the widgets to it. */
  Gtk::VBox* dialog_vbox = MK_VBOX;
  dialog_vbox->pack_start(this->rb_whole_plan, false, false, 0);
  dialog_vbox->pack_start(this->rb_partial_plan, false, false, 0);
  dialog_vbox->pack_start(this->skill_selection, false, false, 0);

  /* Create the button for the next page. */
  Gtk::Button* calculate_but = MK_BUT0;
  calculate_but->set_image(*MK_IMG(Gtk::Stock::MEDIA_PLAY,
      Gtk::ICON_SIZE_BUTTON));
  calculate_but->set_label("Optimize attributes");

  /* The button box. */
  Gtk::HBox* button_box = MK_HBOX;
  button_box->pack_end(*calculate_but, false, false, 0);

  /* The main box. */
  Gtk::VBox* main_box = MK_VBOX;
  main_box->set_border_width(5);
  main_box->pack_start(*info_box, false, false, 0);
  main_box->pack_start(*MK_HSEP, false, false, 0);
  main_box->pack_start(*dialog_vbox, false, false, 0);
  main_box->pack_end(*button_box, false, false, 0);

  /* Add signal handlers to the radio buttons. */
  this->rb_whole_plan.signal_clicked().connect(sigc::bind(sigc::mem_fun
      (*this, &GuiPlanAttribOpt::set_selection_sensitivity), false));
  this->rb_partial_plan.signal_clicked().connect(sigc::bind(sigc::mem_fun
      (*this, &GuiPlanAttribOpt::set_selection_sensitivity), true));
  calculate_but->signal_clicked().connect(sigc::mem_fun
      (*this, &GuiPlanAttribOpt::on_calculate_clicked));

  return main_box;
}

/* ---------------------------------------------------------------- */

Gtk::Widget*
GuiPlanAttribOpt::create_attrib_page (void)
{
  /* Create labels for the attributes' names. */
  Gtk::Label* cha_name_label = MK_LABEL("Charisma:");
  Gtk::Label* intl_name_label = MK_LABEL("Intelligence:");
  Gtk::Label* per_name_label = MK_LABEL("Perception:");
  Gtk::Label* mem_name_label = MK_LABEL("Memory:");
  Gtk::Label* wil_name_label = MK_LABEL("Willpower:");

  /* Create the labels for the times' names. */
  Gtk::Label* best_time_name_label = MK_LABEL("Best time:");
  Gtk::Label* original_time_name_label = MK_LABEL("Original time:");
  Gtk::Label* difference_time_name_label = MK_LABEL("Improvement:");

  /* Create info and warning box. */
  Gtk::Image* info_image = MK_IMG(Gtk::Stock::DIALOG_INFO,
      Gtk::ICON_SIZE_DIALOG);
  Gtk::Label* information_label = MK_LABEL("Below you find the attribute "
      "point distribution which will reduce the current plan's training time "
      "the most.\n"
      "Note: You can only remap your attributes once a year!");
  information_label->set_line_wrap(true);
  information_label->set_justify(Gtk::JUSTIFY_LEFT);
  information_label->set_alignment(Gtk::ALIGN_LEFT);

  Gtk::HBox* info_box = MK_HBOX;
  info_box->set_spacing(10);
  info_box->pack_start(*info_image, false, false, 0);
  info_box->pack_start(*information_label, true, true, 0);

  Gtk::Image* warning_image = MK_IMG(Gtk::Stock::DIALOG_WARNING,
      Gtk::ICON_SIZE_DIALOG);
  Gtk::Label* warning_label = MK_LABEL("The optimized plan's training time "
      "is below one year!\nPlease note that you're allowed to remap your "
      "attributes only once a year.");
  warning_label->set_line_wrap(true);
  warning_label->set_justify(Gtk::JUSTIFY_LEFT);
  warning_label->set_alignment(Gtk::ALIGN_LEFT);

  warning_label->show();
  warning_image->show();
  this->warning_box.set_spacing(10);
  this->warning_box.pack_start(*warning_image, false, false, 0);
  this->warning_box.pack_start(*warning_label, true, true, 0);
  this->warning_box.set_no_show_all(true);

  /* Set the alignments for all the attributes' labels. */
  cha_name_label->set_alignment(Gtk::ALIGN_LEFT);
  intl_name_label->set_alignment(Gtk::ALIGN_LEFT);
  per_name_label->set_alignment(Gtk::ALIGN_LEFT);
  mem_name_label->set_alignment(Gtk::ALIGN_LEFT);
  wil_name_label->set_alignment(Gtk::ALIGN_LEFT);

  this->base_cha_label.set_alignment(Gtk::ALIGN_RIGHT);
  this->base_intl_label.set_alignment(Gtk::ALIGN_RIGHT);
  this->base_per_label.set_alignment(Gtk::ALIGN_RIGHT);
  this->base_mem_label.set_alignment(Gtk::ALIGN_RIGHT);
  this->base_wil_label.set_alignment(Gtk::ALIGN_RIGHT);

  this->total_cha_label.set_alignment(Gtk::ALIGN_RIGHT);
  this->total_intl_label.set_alignment(Gtk::ALIGN_RIGHT);
  this->total_per_label.set_alignment(Gtk::ALIGN_RIGHT);
  this->total_mem_label.set_alignment(Gtk::ALIGN_RIGHT);
  this->total_wil_label.set_alignment(Gtk::ALIGN_RIGHT);

  /* Set the alignments for all the times' labels. */
  best_time_name_label->set_alignment(Gtk::ALIGN_LEFT);
  original_time_name_label->set_alignment(Gtk::ALIGN_LEFT);
  difference_time_name_label->set_alignment(Gtk::ALIGN_LEFT);

  this->best_time_label.set_alignment(Gtk::ALIGN_RIGHT);
  this->original_time_label.set_alignment(Gtk::ALIGN_RIGHT);
  this->difference_time_label.set_alignment(Gtk::ALIGN_RIGHT);

  /* Configure the table and add fill it with content. */
  Gtk::Label* base_attr_label = MK_LABEL("Base");
  base_attr_label->set_alignment(Gtk::ALIGN_RIGHT);
  Gtk::Label* total_attr_label = MK_LABEL("Total");
  total_attr_label->set_alignment(Gtk::ALIGN_RIGHT);

  Gtk::Table* attribute_table = MK_TABLE(9, 3);
  attribute_table->set_col_spacings(10);
  attribute_table->set_row_spacing(5, 10);

  attribute_table->attach(*base_attr_label, 1, 2, 0, 1, Gtk::FILL);
  attribute_table->attach(*total_attr_label, 2, 3, 0, 1, Gtk::FILL);
  attribute_table->attach(*cha_name_label, 0, 1, 1, 2, Gtk::FILL);
  attribute_table->attach(this->base_cha_label, 1, 2, 1, 2, Gtk::FILL);
  attribute_table->attach(this->total_cha_label, 2, 3, 1, 2, Gtk::FILL);
  attribute_table->attach(*intl_name_label, 0, 1, 2, 3, Gtk::FILL);
  attribute_table->attach(this->base_intl_label, 1, 2, 2, 3, Gtk::FILL);
  attribute_table->attach(this->total_intl_label, 2, 3, 2, 3, Gtk::FILL);
  attribute_table->attach(*per_name_label, 0, 1, 3, 4, Gtk::FILL);
  attribute_table->attach(this->base_per_label, 1, 2, 3, 4, Gtk::FILL);
  attribute_table->attach(this->total_per_label, 2, 3, 3, 4, Gtk::FILL);
  attribute_table->attach(*mem_name_label, 0, 1, 4, 5, Gtk::FILL);
  attribute_table->attach(this->base_mem_label, 1, 2, 4, 5, Gtk::FILL);
  attribute_table->attach(this->total_mem_label, 2, 3, 4, 5, Gtk::FILL);
  attribute_table->attach(*wil_name_label, 0, 1, 5, 6, Gtk::FILL);
  attribute_table->attach(this->base_wil_label, 1, 2, 5, 6, Gtk::FILL);
  attribute_table->attach(this->total_wil_label, 2, 3, 5, 6, Gtk::FILL);

  attribute_table->attach(*original_time_name_label, 0, 1, 6, 7, Gtk::FILL);
  attribute_table->attach(this->original_time_label, 1, 3, 6, 7, Gtk::FILL);
  attribute_table->attach(*best_time_name_label, 0, 1, 7, 8, Gtk::FILL);
  attribute_table->attach(this->best_time_label, 1, 3, 7, 8, Gtk::FILL);
  attribute_table->attach(*difference_time_name_label, 0, 1, 8, 9, Gtk::FILL);
  attribute_table->attach(this->difference_time_label, 1, 3, 8, 9, Gtk::FILL);

  /* Configure some buttons. */
  Gtk::Button* view_plan_but = MK_BUT0;
  view_plan_but->set_image(*MK_IMG(Gtk::Stock::MEDIA_PLAY,
      Gtk::ICON_SIZE_BUTTON));
  view_plan_but->set_label("View plan breakdown");

  Gtk::HBox* view_plan_box = MK_HBOX;
  view_plan_box->pack_end(*view_plan_but, false, false, 0);

  /* Assemble and configure the widgets. */
  Gtk::VBox* attribute_box = MK_VBOX;
  attribute_box->set_border_width(5);
  attribute_box->pack_start(*info_box, false, false);
  attribute_box->pack_start(*MK_HSEP, false, false, 0);
  attribute_box->pack_start(*attribute_table, false, false, 10);
  attribute_box->pack_start(this->warning_box, false, false);
  attribute_box->pack_end(*view_plan_box, false, false, 0);

  /* Signals. */
  view_plan_but->signal_clicked().connect(sigc::bind(sigc::mem_fun
      (this->notebook, &Gtk::Notebook::set_current_page), 2));

  return attribute_box;
}

/* ---------------------------------------------------------------- */

Gtk::Widget*
GuiPlanAttribOpt::create_breakdown_page (void)
{
  /* Configure the skill list. */
  Gtk::ScrolledWindow *scwin = MK_SCWIN;
  scwin->set_border_width(5);
  scwin->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_ALWAYS);
  scwin->set_shadow_type(Gtk::SHADOW_ETCHED_IN);
  scwin->add(this->treeview);

  this->viewcols.set_format("-0 +1 -2 +3 -4 -5 -6 -7 -8 -9");
  this->viewcols.setup_columns_normal();
  this->treeview.set_rules_hint(true);

  return scwin;
}

/* ---------------------------------------------------------------- */

void
GuiPlanAttribOpt::set_plan (GtkSkillList const& plan)
{
  this->plan = plan;

  /* Fill the skill selection. */
  for (unsigned int i = 0; i < this->plan.size(); i++)
  {
    GtkSkillInfo& info = this->plan[i];
    Glib::ustring skillname = info.skill->name;
    skillname += " " + Helpers::get_roman_from_int(info.plan_level);
    this->skill_selection.append_text(skillname);
  }
  this->skill_selection.set_active(0);
}

/* ---------------------------------------------------------------- */

void
GuiPlanAttribOpt::on_calculate_clicked (void)
{
  this->notebook.set_current_page(1);
  if (this->rb_partial_plan.get_active())
  {
    this->plan_offset = this->skill_selection.get_active_row_number();
  }
  else
  {
    this->plan_offset = 0;
  }
  this->optimize_plan();
}

/* ---------------------------------------------------------------- */

void
GuiPlanAttribOpt::set_selection_sensitivity (bool sensitive)
{
  this->skill_selection.set_sensitive(sensitive);
}

/* ---------------------------------------------------------------- */

void
GuiPlanAttribOpt::optimize_plan (void)
{
  /* Copy the original plan because it may be altered later. */
  GtkSkillList plan_part = this->plan;

  /* Fetch the character from the plan. */
  ApiCharSheetPtr charsheet = this->plan.get_character()->cs;

  /* Fetch base and implant attribute points. */
  ApiCharAttribs base_atts = charsheet->base;
  ApiCharAttribs implant_atts = charsheet->implant;
  ApiCharAttribs total_atts = charsheet->total;

  if (this->plan_offset > 0)
    plan_part.erase(plan_part.begin(), plan_part.begin() + this->plan_offset);

  /* Copy the possibly cleaned plan to have an original one for comparison. */
  GtkSkillList plan_orig = plan_part;

  /* Use the current total time and base attributes as base. */
  ApiCharAttribs cur_base_atts = base_atts;
  ApiCharAttribs cur_total_atts = total_atts;
  ApiCharAttribs best_base_atts = base_atts;
  ApiCharAttribs best_total_atts = total_atts;

  /* Calculate the original plan with the original attribues. */
  plan_orig.calc_details(cur_total_atts);
  cur_total_atts = total_atts;

  time_t orig_total_time = plan_orig.back().train_duration;
  time_t cur_total_time = orig_total_time;
  time_t best_total_time = orig_total_time;

  /* Calculate the maximum number of points that can be assigned to each
   * attribute. */
  int max_points_per_att = MAXIMUM_VALUE_PER_ATTRIB
      - MINIMUM_VALUE_PER_ATTRIB;

  /* Calculate the total number of base attribute points to distribute if it
   * changes in the future. */
  int total_base_atts = (int)cur_base_atts.cha + (int)cur_base_atts.intl
      + (int)cur_base_atts.mem + (int)cur_base_atts.per
      + (int)cur_base_atts.wil - (MINIMUM_VALUE_PER_ATTRIB * 5);


  /* Go through all combinations and compare the runtime.
   * This algorithm has been found in EVEMon.
   *
   * This is O(scary), but seems quick enough in practice.
   */
  for (int intl = 0; intl <= max_points_per_att; intl++)
  {
    int max_mem = total_base_atts - intl;
    for (int mem = 0; mem <= max_points_per_att && mem <= max_mem; mem++)
    {
      int max_cha = max_mem - mem;
      for (int cha = 0; cha <= max_points_per_att && cha <= max_cha; cha++)
      {
        int max_per = max_cha - cha;
        for (int per = 0; per <= max_points_per_att && per <= max_per; per++)
        {
          int wil = max_per - per;
          if (wil <= max_points_per_att)
          {
            /* Calculate the base attributes based on the current
             * values. */
            cur_base_atts.intl = intl + MINIMUM_VALUE_PER_ATTRIB;
            cur_base_atts.mem = mem + MINIMUM_VALUE_PER_ATTRIB;
            cur_base_atts.cha = cha + MINIMUM_VALUE_PER_ATTRIB;
            cur_base_atts.per = per + MINIMUM_VALUE_PER_ATTRIB;
            cur_base_atts.wil = wil + MINIMUM_VALUE_PER_ATTRIB;

            /* Calculate the total attributes based on the
             * current base attributes and the fetched learning skills
             * and implants. */
            cur_total_atts = cur_base_atts + implant_atts;

            ApiCharAttribs cur_total_atts_copy = cur_total_atts;
            plan_part.calc_details(cur_total_atts_copy);
            cur_total_time = plan_part.back().train_duration;

            if (cur_total_time < best_total_time)
            {
              best_total_time = cur_total_time;
              best_base_atts = cur_base_atts;
              best_total_atts = cur_total_atts;
            }
          }
        }
      }
    }
  }

  /* Calculate the details for the new list with the best attributes. */
  {
    ApiCharAttribs best_total_atts_copy = best_total_atts;
    plan_part.calc_details(best_total_atts_copy);
  }

  /* Warn the user if the optimized time is below one year. */
  if (best_total_time < (365 * 24 * 60 * 60))
    this->warning_box.show();
  else
    this->warning_box.hide();

  /* Write the best base attributes and the training times into the labels. */
  this->base_cha_label.set_text(Helpers::get_string_from_double
      (best_base_atts.cha, 0));
  this->base_intl_label.set_text(Helpers::get_string_from_double
      (best_base_atts.intl, 0));
  this->base_per_label.set_text(Helpers::get_string_from_double
      (best_base_atts.per, 0));
  this->base_mem_label.set_text(Helpers::get_string_from_double
      (best_base_atts.mem, 0));
  this->base_wil_label.set_text(Helpers::get_string_from_double
      (best_base_atts.wil, 0));

  this->total_cha_label.set_text(Helpers::get_string_from_double
      (best_total_atts.cha, 2));
  this->total_intl_label.set_text(Helpers::get_string_from_double
      (best_total_atts.intl, 2));
  this->total_per_label.set_text(Helpers::get_string_from_double
      (best_total_atts.per, 2));
  this->total_mem_label.set_text(Helpers::get_string_from_double
      (best_total_atts.mem, 2));
  this->total_wil_label.set_text(Helpers::get_string_from_double
      (best_total_atts.wil, 2));

  original_time_label.set_text(EveTime::get_string_for_timediff
      (orig_total_time, false));
  best_time_label.set_text(EveTime::get_string_for_timediff
      (best_total_time, false));
  difference_time_label.set_text(EveTime::get_string_for_timediff
      (orig_total_time - best_total_time, false));

  /* Fill the skill list with data. Comment out those that aren't needed. */
  this->liststore->clear();
  for (unsigned int i = 0; i < plan_part.size(); ++i)
  {
    GtkSkillInfo& info = plan_part[i];
    ApiSkill const* skill = info.skill;

    Gtk::ListStore::iterator iter = this->liststore->append();

    Glib::ustring skillname = skill->name;
    skillname += " " + Helpers::get_roman_from_int(info.plan_level);
    skillname += "  (" + Helpers::get_string_from_int(skill->rank) + ")";

    (*iter)[this->cols.skill_index] = i;
    (*iter)[this->cols.skill_name] = skillname;
    (*iter)[this->cols.skill_icon] = ImageStore::skillplan[info.skill_icon];
    (*iter)[this->cols.skill_duration]
        = EveTime::get_string_for_timediff(info.skill_duration, true);

    /* Calculate the duration difference between the
     * old attributes and the optimized ones. */
    time_t difference = info.skill_duration - plan_orig[i].skill_duration;
    if (difference < 0)
    {
      (*iter)[this->cols.difference]
          = "- " + EveTime::get_string_for_timediff(-difference, true);
    }
    else
    {
      (*iter)[this->cols.difference]
          = "+ " + EveTime::get_string_for_timediff(difference, true);
    }
  }
}
