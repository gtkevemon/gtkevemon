#include <iostream>
#include <sstream>
#include <gtkmm/main.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
#include <gtkmm/separator.h>
#include <gtkmm/table.h>
#include <gtkmm/box.h>
#include <gtkmm/treeview.h>
#include <gtkmm/image.h>
#include <gtkmm/stock.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/alignment.h>

#include "util/helpers.h"
#include "util/exception.h"
#include "api/evetime.h"
#include "api/apicharsheet.h"
#include "api/apiskilltree.h"
#include "bits/config.h"
#include "bits/notifier.h"
#include "bits/characterlist.h"

#include "imagestore.h"
#include "gtkdefines.h"
#include "gtkhelpers.h"
#include "guiskill.h"
#include "guiskillqueue.h"
#include "gtkcharpage.h"

GtkCharPage::GtkCharPage (CharacterPtr character)
  : Gtk::VBox(false, 5),
    character(character),
    info_display(INFO_STYLE_TOP_HSEP)
{
  /* Setup GUI. */
  this->char_image.set_enable_clicks();

  this->refresh_but.set_image(*Gtk::manage(new Gtk::Image
      (Gtk::Stock::REFRESH, Gtk::ICON_SIZE_MENU)));
  this->refresh_but.set_relief(Gtk::RELIEF_NONE);
  this->refresh_but.set_focus_on_click(false);

  this->info_but.set_image(*Gtk::manage(new Gtk::Image
      (Gtk::Stock::INFO, Gtk::ICON_SIZE_MENU)));
  this->info_but.set_relief(Gtk::RELIEF_NONE);

  this->char_name_label.set_alignment(Gtk::ALIGN_LEFT);
  this->char_info_label.set_alignment(Gtk::ALIGN_LEFT);
  this->corp_label.set_alignment(Gtk::ALIGN_LEFT);
  this->balance_label.set_alignment(Gtk::ALIGN_LEFT);
  this->skill_points_label.set_alignment(Gtk::ALIGN_LEFT);
  this->known_skills_label.set_alignment(Gtk::ALIGN_LEFT);
  this->attr_cha_label.set_alignment(Gtk::ALIGN_LEFT);
  this->attr_int_label.set_alignment(Gtk::ALIGN_LEFT);
  this->attr_per_label.set_alignment(Gtk::ALIGN_LEFT);
  this->attr_mem_label.set_alignment(Gtk::ALIGN_LEFT);
  this->attr_wil_label.set_alignment(Gtk::ALIGN_LEFT);
  this->training_label.set_alignment(Gtk::ALIGN_LEFT);
  this->remaining_label.set_alignment(Gtk::ALIGN_LEFT);
  this->finish_eve_label.set_alignment(Gtk::ALIGN_LEFT);
  this->finish_local_label.set_alignment(Gtk::ALIGN_LEFT);
  this->spph_label.set_alignment(Gtk::ALIGN_RIGHT);
  this->live_sp_label.set_alignment(Gtk::ALIGN_RIGHT);

  this->charsheet_info_label.set_alignment(Gtk::ALIGN_RIGHT);
  this->skillqueue_info_label.set_alignment(Gtk::ALIGN_RIGHT);

  /* Setup skill list. */
  Gtk::TreeViewColumn* name_column = Gtk::manage(new Gtk::TreeViewColumn);
  name_column->set_title("Skill name (rank)");
  name_column->pack_start(this->skill_cols.icon, false);
  #ifdef GLIBMM_PROPERTIES_ENABLED
  Gtk::CellRendererText* name_renderer = Gtk::manage(new Gtk::CellRendererText);
  name_column->pack_start(*name_renderer, true);
  name_column->add_attribute(name_renderer->property_markup(),
      this->skill_cols.name);
  #else
  /* FIXME: Activate markup here. */
  name_column->pack_start(this->skill_cols.name);
  #endif

  this->skill_store = Gtk::TreeStore::create(this->skill_cols);
  this->skill_store->set_sort_column
      (this->skill_cols.name, Gtk::SORT_ASCENDING);
  this->skill_view.set_model(this->skill_store);
  this->skill_view.set_rules_hint(true);
  this->skill_view.append_column(*name_column);
  this->skill_view.append_column("Level", this->skill_cols.level);
  this->skill_view.append_column("Points", this->skill_cols.points);
  this->skill_view.append_column("Max. Points", this->skill_cols.max_points);
  this->skill_view.append_column("Pri.", this->skill_cols.primary);
  this->skill_view.append_column("Sec.", this->skill_cols.secondary);
  this->skill_view.get_column(0)->set_expand(true);
  this->skill_view.get_column(2)->get_first_cell_renderer()
      ->set_property("xalign", 1.0f);
  this->skill_view.get_column(3)->get_first_cell_renderer()
      ->set_property("xalign", 1.0f);

  //this->skill_view.set_grid_lines(Gtk::TREE_VIEW_GRID_LINES_BOTH);

  /* Build GUI elements. */
  Gtk::ScrolledWindow* scwin = Gtk::manage(new Gtk::ScrolledWindow);
  scwin->set_shadow_type(Gtk::SHADOW_ETCHED_IN);
  scwin->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_ALWAYS);
  scwin->add(this->skill_view);
  scwin->show_all();

  Gtk::Table* info_table = Gtk::manage(new Gtk::Table(5, 7));
  info_table->set_col_spacings(10);
  Gtk::Label* corp_desc = MK_LABEL("Corporation:");
  Gtk::Label* isk_desc = MK_LABEL("Balance:");
  Gtk::Label* skillpoints_desc = MK_LABEL("Skill points:");
  Gtk::Label* knownskills_desc = MK_LABEL("Known skills:");
  Gtk::Label* attr_charisma_desc = MK_LABEL("Charisma:");
  Gtk::Label* attr_intelligence_desc = MK_LABEL("Intelligence:");
  Gtk::Label* attr_perception_desc = MK_LABEL("Perception:");
  Gtk::Label* attr_memory_desc = MK_LABEL("Memory:");
  Gtk::Label* attr_willpower_desc = MK_LABEL("Willpower:");
  corp_desc->set_alignment(Gtk::ALIGN_LEFT);
  isk_desc->set_alignment(Gtk::ALIGN_LEFT);
  skillpoints_desc->set_alignment(Gtk::ALIGN_LEFT);
  knownskills_desc->set_alignment(Gtk::ALIGN_LEFT);
  attr_charisma_desc->set_alignment(Gtk::ALIGN_LEFT);
  attr_intelligence_desc->set_alignment(Gtk::ALIGN_LEFT);
  attr_perception_desc->set_alignment(Gtk::ALIGN_LEFT);
  attr_memory_desc->set_alignment(Gtk::ALIGN_LEFT);
  attr_willpower_desc->set_alignment(Gtk::ALIGN_LEFT);

  Gtk::Button* close_but = MK_BUT0;
  close_but->set_relief(Gtk::RELIEF_NONE);
  close_but->set_image(*Gtk::manage(new Gtk::Image
      (Gtk::Stock::CLOSE, Gtk::ICON_SIZE_MENU)));

  Gtk::VBox* char_buts_vbox = MK_VBOX0;
  char_buts_vbox->pack_start(*close_but, false, false, 0);
  //char_buts_vbox->pack_start(*MK_HSEP, true, true, 0);
  char_buts_vbox->pack_end(this->refresh_but, false, false, 0);
  char_buts_vbox->pack_end(this->info_but, false, false, 0);
  Gtk::HBox* char_buts_hbox = MK_HBOX;
  char_buts_hbox->pack_end(*char_buts_vbox, false, false, 0);

  /* Character SP */
  Gtk::HBox* char_skillpoints_box = MK_HBOX;
  char_skillpoints_box->pack_start(this->skill_points_label, false, false, 0);

  info_table->attach(this->char_image, 0, 1, 0, 5, Gtk::SHRINK, Gtk::SHRINK);
  info_table->attach(this->char_name_label, 1, 2, 0, 1, Gtk::FILL, Gtk::FILL);
  info_table->attach(*corp_desc, 1, 2, 1, 2, Gtk::FILL, Gtk::FILL);
  info_table->attach(*isk_desc, 1, 2, 2, 3, Gtk::FILL, Gtk::FILL);
  info_table->attach(*skillpoints_desc, 1, 2, 3, 4, Gtk::FILL, Gtk::FILL);
  info_table->attach(*knownskills_desc, 1, 2, 4, 5, Gtk::FILL, Gtk::FILL);
  info_table->attach(this->char_info_label, 2, 3, 0, 1, Gtk::FILL, Gtk::FILL);
  info_table->attach(this->corp_label, 2, 3, 1, 2, Gtk::FILL, Gtk::FILL);
  info_table->attach(this->balance_label, 2, 3, 2, 3, Gtk::FILL, Gtk::FILL);
  info_table->attach(*char_skillpoints_box, 2, 3, 3, 4, Gtk::FILL, Gtk::FILL);
  info_table->attach(this->known_skills_label, 2, 3, 4, 5,
      Gtk::FILL, Gtk::FILL);
  info_table->attach(*MK_VSEP, 3, 4, 0, 5, Gtk::FILL, Gtk::FILL);
  info_table->attach(*attr_charisma_desc, 4, 5, 0, 1, Gtk::FILL, Gtk::FILL);
  info_table->attach(*attr_intelligence_desc, 4, 5, 1, 2, Gtk::FILL, Gtk::FILL);
  info_table->attach(*attr_perception_desc, 4, 5, 2, 3, Gtk::FILL, Gtk::FILL);
  info_table->attach(*attr_memory_desc, 4, 5, 3, 4, Gtk::FILL, Gtk::FILL);
  info_table->attach(*attr_willpower_desc, 4, 5, 4, 5, Gtk::FILL, Gtk::FILL);
  info_table->attach(this->attr_cha_label, 5, 6, 0, 1, Gtk::FILL, Gtk::FILL);
  info_table->attach(this->attr_int_label, 5, 6, 1, 2, Gtk::FILL, Gtk::FILL);
  info_table->attach(this->attr_per_label, 5, 6, 2, 3, Gtk::FILL, Gtk::FILL);
  info_table->attach(this->attr_mem_label, 5, 6, 3, 4, Gtk::FILL, Gtk::FILL);
  info_table->attach(this->attr_wil_label, 5, 6, 4, 5, Gtk::FILL, Gtk::FILL);
  info_table->attach(*char_buts_hbox, 6, 7, 0, 5,
      Gtk::FILL | Gtk::EXPAND, Gtk::SHRINK | Gtk::FILL);

  /* Prepare training table. */
  Gtk::Label* train_desc = MK_LABEL("<b>Training:</b>");
  Gtk::Label* remain_desc = MK_LABEL("Remaining:");
  Gtk::Label* finish_eve_desc = MK_LABEL("Finish (EVE time):");
  Gtk::Label* finish_local_desc = MK_LABEL("Finish (local time):");
  train_desc->set_use_markup(true);
  train_desc->set_alignment(Gtk::ALIGN_LEFT);
  remain_desc->set_alignment(Gtk::ALIGN_LEFT);
  finish_eve_desc->set_alignment(Gtk::ALIGN_LEFT);
  finish_local_desc->set_alignment(Gtk::ALIGN_LEFT);

  Gtk::Label* charsheet_info_desc = MK_LABEL("Character sheet:");
  Gtk::Label* trainsheet_info_desc = MK_LABEL("Skill queue:");
  Gtk::HBox* charsheet_info_hbox = MK_HBOX;
  charsheet_info_hbox->pack_end(this->charsheet_info_label, false, false, 0);
  charsheet_info_hbox->pack_end(*charsheet_info_desc, false, false, 0);
  Gtk::HBox* trainsheet_info_hbox = MK_HBOX;
  trainsheet_info_hbox->pack_end(this->skillqueue_info_label, false, false, 0);
  trainsheet_info_hbox->pack_end(*trainsheet_info_desc, false, false, 0);

  /* Setup training table. */
  Gtk::Button* skillqueue_but = MK_BUT0;
  skillqueue_but->add(*MK_IMG_PB(ImageStore::skillqueue));
  skillqueue_but->set_relief(Gtk::RELIEF_NONE);
  skillqueue_but->set_tooltip_text("Show training queue");
  skillqueue_but->set_focus_on_click(false);

  Gtk::Table* train_sub_tbl = MK_TABLE(2, 2);
  train_sub_tbl->set_col_spacings(5);
  train_sub_tbl->attach(*skillqueue_but, 0, 1, 0, 2, Gtk::SHRINK, Gtk::SHRINK);
  train_sub_tbl->attach(*train_desc, 1, 2, 0, 1, Gtk::FILL | Gtk::EXPAND);
  train_sub_tbl->attach(*remain_desc, 1, 2, 1, 2, Gtk::FILL | Gtk::EXPAND);

  Gtk::Table* train_table = MK_TABLE(4, 3);
  train_table->set_col_spacings(10);
  train_table->attach(*train_sub_tbl, 0, 1, 0, 2, Gtk::FILL, Gtk::FILL);
  train_table->attach(*finish_eve_desc, 0, 1, 2, 3, Gtk::FILL, Gtk::FILL);
  train_table->attach(*finish_local_desc, 0, 1, 3, 4, Gtk::FILL, Gtk::FILL);
  train_table->attach(this->training_label, 1, 2, 0, 1, Gtk::FILL);
  train_table->attach(this->remaining_label, 1, 2, 1, 2, Gtk::FILL);
  train_table->attach(this->finish_eve_label, 1, 2, 2, 3, Gtk::FILL);
  train_table->attach(this->finish_local_label, 1, 2, 3, 4, Gtk::FILL);
  train_table->attach(this->spph_label, 2, 3, 2, 3,
      Gtk::FILL | Gtk::EXPAND, Gtk::SHRINK);
  train_table->attach(this->live_sp_label, 2, 3, 3, 4,
      Gtk::FILL | Gtk::EXPAND, Gtk::SHRINK);
  train_table->attach(*charsheet_info_hbox, 2, 3, 0, 1,
      Gtk::FILL | Gtk::EXPAND, Gtk::FILL);
  train_table->attach(*trainsheet_info_hbox, 2, 3, 1, 2,
      Gtk::FILL | Gtk::EXPAND, Gtk::FILL);

  /* Global packing. */
  this->set_border_width(5);
  this->pack_start(*info_table, false, false, 0);
  this->pack_start(*scwin, true, true, 0);
  this->pack_start(*train_table, false, false, 0);
  this->pack_start(this->info_display, false, false, 0);

  /* Setup tooltips. */
  close_but->set_tooltip_text("Close the character");
  this->info_but.set_tooltip_text("Infomation about cached sheets");
  this->refresh_but.set_tooltip_text("Request API information");

  /* Signals. */
  close_but->signal_clicked().connect(sigc::mem_fun
      (*this, &GtkCharPage::on_close_clicked));
  skillqueue_but->signal_clicked().connect(sigc::mem_fun
      (*this, &GtkCharPage::on_skillqueue_clicked));
  this->refresh_but.signal_clicked().connect(sigc::mem_fun
      (*this, &GtkCharPage::request_documents));
  this->info_but.signal_clicked().connect(sigc::mem_fun
      (*this, &GtkCharPage::on_info_clicked));
  this->skill_view.signal_row_activated().connect(sigc::mem_fun
      (*this, &GtkCharPage::on_skill_activated));
  this->skill_view.set_has_tooltip(true);
  this->skill_view.signal_query_tooltip().connect(sigc::mem_fun
      (*this, &GtkCharPage::on_query_skillview_tooltip));

  this->character->signal_api_info_changed().connect
      (sigc::mem_fun(*this, &GtkCharPage::api_info_changed));
  this->character->signal_skill_completed().connect
      (sigc::mem_fun(*this, &GtkCharPage::on_skill_completed));
  this->character->signal_request_error().connect(sigc::bind
      (sigc::mem_fun(*this, &GtkCharPage::on_api_error), false));
  this->character->signal_cached_warning().connect(sigc::bind
      (sigc::mem_fun(*this, &GtkCharPage::on_api_error), true));
  this->character->signal_training_changed().connect
      (sigc::mem_fun(*this, &GtkCharPage::update_training_details));

  Glib::signal_timeout().connect(sigc::mem_fun(*this,
      &GtkCharPage::on_live_sp_value_update), CHARPAGE_LIVE_SP_LABEL_UPDATE);
  Glib::signal_timeout().connect(sigc::mem_fun(*this,
      &GtkCharPage::on_live_sp_image_update), CHARPAGE_LIVE_SP_IMAGE_UPDATE);
  Glib::signal_timeout().connect(sigc::mem_fun(*this,
      &GtkCharPage::check_expired_sheets), CHARPAGE_CHECK_EXPIRED_SHEETS);
  Glib::signal_timeout().connect(sigc::mem_fun(*this,
      &GtkCharPage::update_cached_duration), CHARPAGE_UPDATE_CACHED_DURATION);

  /* Request data update and update GUI. */
  this->request_documents();
  this->char_image.set(this->character->get_char_id());
  this->update_charsheet_details();
  this->update_training_details();

  this->show_all();
  this->info_display.hide();
}

/* ---------------------------------------------------------------- */

void
GtkCharPage::update_charsheet_details (void)
{
  ApiCharSheetPtr cs = this->character->cs;
  ApiSkillTreePtr tree = ApiSkillTree::request();

  this->char_name_label.set_text("<b>"
      + this->character->get_char_name() + "</b>");
  this->char_name_label.set_use_markup(true);

  /* Set character information. */
  if (!cs->valid)
  {
    this->char_info_label.set_text("---");
    this->corp_label.set_text("---");
    this->balance_label.set_text("---");
    this->skill_points_label.set_text("---");
    this->known_skills_label.set_text("---");
    this->attr_cha_label.set_text("---");
    this->attr_int_label.set_text("---");
    this->attr_per_label.set_text("---");
    this->attr_mem_label.set_text("---");
    this->attr_wil_label.set_text("---");

    this->known_skills_label.set_has_tooltip(false);
    this->skill_points_label.set_has_tooltip(false);
    this->attr_cha_label.set_has_tooltip(false);
  }
  else
  {
    /* Load some configuration. */
    ConfValuePtr trunc_corpname = Config::conf.get_value
        ("settings.trunc_corpname");

    /* Set character labels. */
    this->char_info_label.set_text(cs->gender + ", "
        + cs->race + ", " + cs->bloodline);
    this->balance_label.set_text(Helpers::get_dotted_isk(cs->balance) + " ISK");

    if (trunc_corpname->get_bool())
    {
      this->corp_label.set_text(Helpers::trunc_string(cs->corp, 25));
      this->corp_label.set_tooltip_text(cs->corp);
    }
    else
      this->corp_label.set_text(cs->corp);

    this->skill_points_label.set_text(Helpers::get_dotted_str_from_uint
        (cs->total_sp));
    this->known_skills_label.set_text(Helpers::get_string_from_sizet
        (cs->skills.size()) + " known skills ("
        + Helpers::get_string_from_uint(cs->skills_at[5])
        + " at V)");
        // "of " + Helpers::get_string_from_int(tree->count_total_skills()) + " total");

    this->attr_cha_label.set_text(Helpers::get_string_from_double
        (cs->total.cha, 2));
    this->attr_int_label.set_text(Helpers::get_string_from_double
        (cs->total.intl, 2));
    this->attr_per_label.set_text(Helpers::get_string_from_double
        (cs->total.per, 2));
    this->attr_mem_label.set_text(Helpers::get_string_from_double
        (cs->total.mem, 2));
    this->attr_wil_label.set_text(Helpers::get_string_from_double
        (cs->total.wil, 2));

    /* Build list of known skills per level (tooltip). */
    Glib::ustring skills_at_tt;
    skills_at_tt = "<u><b>List of known skills</b></u>\n";
    for (int i = 0; i < 6; ++i)
    {
      skills_at_tt += "Level ";
      skills_at_tt += Helpers::get_string_from_uint(i);
      skills_at_tt += ": ";
      skills_at_tt += Helpers::get_string_from_uint
          (cs->skills_at[i]);
      if (i != 5)
        skills_at_tt += "\n";
    }

    /* Build detailed attribute information (tooltip). */
    Glib::ustring attr_cha_tt;
    attr_cha_tt = "<u><b>Attribute: Charisma</b></u>\nBase: ";
    attr_cha_tt += Helpers::get_string_from_double(cs->base.cha, 2);
    attr_cha_tt += "\nImplants: ";
    attr_cha_tt += Helpers::get_string_from_double(cs->implant.cha, 2);

    Glib::ustring attr_int_tt;
    attr_int_tt = "<u><b>Attribute: Intelligence</b></u>\nBase: ";
    attr_int_tt += Helpers::get_string_from_double(cs->base.intl, 2);
    attr_int_tt += "\nImplants: ";
    attr_int_tt += Helpers::get_string_from_double(cs->implant.intl,2);

    Glib::ustring attr_per_tt;
    attr_per_tt = "<u><b>Attribute: Perception</b></u>\nBase: ";
    attr_per_tt += Helpers::get_string_from_double(cs->base.per, 2);
    attr_per_tt += "\nImplants: ";
    attr_per_tt += Helpers::get_string_from_double(cs->implant.per, 2);

    Glib::ustring attr_mem_tt;
    attr_mem_tt = "<u><b>Attribute: Memory</b></u>\nBase: ";
    attr_mem_tt += Helpers::get_string_from_double(cs->base.mem, 2);
    attr_mem_tt += "\nImplants: ";
    attr_mem_tt += Helpers::get_string_from_double(cs->implant.mem, 2);

    Glib::ustring attr_wil_tt;
    attr_wil_tt = "<u><b>Attribute: Willpower</b></u>\nBase: ";
    attr_wil_tt += Helpers::get_string_from_double(cs->base.wil, 2);
    attr_wil_tt += "\nImplants: ";
    attr_wil_tt += Helpers::get_string_from_double(cs->implant.wil, 2);

    /* Update some character sheet related skills. */
    this->known_skills_label.set_tooltip_markup(skills_at_tt);
    this->attr_cha_label.set_tooltip_markup(attr_cha_tt);
    this->attr_int_label.set_tooltip_markup(attr_int_tt);
    this->attr_per_label.set_tooltip_markup(attr_per_tt);
    this->attr_mem_label.set_tooltip_markup(attr_mem_tt);
    this->attr_wil_label.set_tooltip_markup(attr_wil_tt);
  }

  this->update_skill_list();
}

/* ---------------------------------------------------------------- */

void
GtkCharPage::update_training_details (void)
{
    if (this->character->is_training())
    {
        time_t end_time_t = this->character->training_info.end_time_t;
        this->training_label.set_text(this->character->get_training_text());

        std::string downtime_str;
        if (EveTime::is_in_eve_downtime(end_time_t))
            downtime_str = " <i>(in DT!)</i>";
        this->finish_eve_label.set_markup(EveTime::get_gm_time_string
            (end_time_t, false) + downtime_str);
        this->finish_local_label.set_markup(EveTime::get_local_time_string
            (EveTime::adjust_local_time(end_time_t), false) + downtime_str);
        this->spph_label.set_text(Helpers::get_string_from_uint
            (this->character->training_spph) + " SP per hour");

        /* Set SP/h tooltip. */
        std::stringstream spph_tooltip;
        spph_tooltip << "<b><u>Training based</u></b>\n"
            << this->character->training_spph << " SP/h";
        if (this->character->cs->valid)
        {
            spph_tooltip << "\n<b><u>Attribute based</u></b>\n"
                << this->character->cs->get_spph_for_skill
                (this->character->training_skill) << " SP/h";
        }
        this->spph_label.set_has_tooltip(true);
        this->spph_label.set_tooltip_markup(spph_tooltip.str());
    }
    else if (this->character->valid_training_sheet())
    {
        this->training_label.set_text("No skill in training!");
        this->remaining_label.set_text("---");
        this->finish_eve_label.set_text("---");
        this->finish_local_label.set_text("---");
        this->spph_label.set_text("0 SP per hour");
        this->spph_label.set_has_tooltip(false);
        this->live_sp_label.set_text("---");
    }
    else
    {
        this->training_label.set_text("---");
        this->remaining_label.set_text("---");
        this->finish_eve_label.set_text("---");
        this->finish_local_label.set_text("---");
        this->spph_label.set_text("---");
        this->spph_label.set_has_tooltip(false);
        this->live_sp_label.set_text("---");
    }
}

/* ---------------------------------------------------------------- */

/* Create a data structure that maps the group ID to the iterator of
 * the group, skill points for that group and the amount of skills.
 * This is needed for building the skill list. */
struct SkillGroupInfo
{
  Gtk::TreeModel::iterator iter;
  int sp;
  bool empty;
  int max;

  SkillGroupInfo (Gtk::TreeModel::iterator iter)
      : iter(iter), sp(0), empty(true) {}
};
typedef std::map<int, SkillGroupInfo> IterMapType;

/* ---------------------------------------------------------------- */

void
GtkCharPage::update_skill_list (void)
{
  this->skill_store->clear();

  if (!this->character->cs->valid)
    return;

  /* Load the skill tree. */
  ApiSkillTreePtr tree;
  try
  {
    tree = ApiSkillTree::request();
  }
  catch (Exception& e)
  {
    this->on_skilltree_error(e);
    return;
  }

  /* Cache skill in training. */
  ApiSkill skill_training;
  skill_training.id = -1;
  skill_training.group = -1;
  if (this->character->is_training() && this->character->training_skill != 0)
    skill_training = *this->character->training_skill;

  /* Append all groups to the store. Save their iterators for the children.
   * Format is <group_id, <model iter, group sp> >. */
  IterMapType iter_map;
  for (ApiSkillGroupMap::iterator iter = tree->groups.begin();
      iter != tree->groups.end(); iter++)
  {
    std::string name = iter->second.name;
    if (skill_training.group == iter->first)
      name += "  <i>(1 in training)</i>";

    Gtk::TreeModel::iterator siter = this->skill_store->append();
    (*siter)[this->skill_cols.id] = -1;
    (*siter)[this->skill_cols.skill] = 0;
    (*siter)[this->skill_cols.name] = name;
    (*siter)[this->skill_cols.icon] = ImageStore::skillicons[0];
    iter_map.insert(std::make_pair(iter->first, SkillGroupInfo(siter)));
  }

  /* Compute max points per skill group. */
  for (ApiSkillMap::iterator iter = tree->skills.begin();
       iter != tree->skills.end(); iter++)
  {
    IterMapType::iterator iiter = iter_map.find(iter->second.group);
    if (iiter != iter_map.end())
    {
      iiter->second.max += ApiCharSheet::calc_dest_sp(4, iter->second.rank);
    }
  }

  /* Append all skills to the skill groups. */
  std::vector<ApiCharSheetSkill>& skills = this->character->cs->skills;
  for (unsigned int i = 0; i < skills.size(); ++i)
  {
    /* Get skill object. */
    ApiSkill const& skill = *skills[i].details;

    /* Lookup skill group. */
    IterMapType::iterator iiter = iter_map.find(skill.group);
    if (iiter == iter_map.end())
    {
      std::cout << "Error appending skill, unknown group!" << std::endl;
      continue;
    }

    /* Append a new row. */
    Gtk::TreeModel::iterator iter = this->skill_store->append
        (iiter->second.iter->children());

    /* Get skill name and set icon. */
    std::string skill_name = skill.name + " ("
        + Helpers::get_string_from_int(skill.rank) + ")";

    if (skill.id == skill_training.id)
    {
      skill_name += "  <i>(in training)</i>";
      (*iter)[this->skill_cols.icon] = ImageStore::skillicons[2];

      /* Update of the SkillInTrainingInfo. */
      this->tree_skill_iter = iter;
    }
    else
    {
      if (skills[i].points != skills[i].points_start)
        (*iter)[this->skill_cols.icon] = ImageStore::skillicons[4];
      else if (skills[i].level < 5)
        (*iter)[this->skill_cols.icon] = ImageStore::skillicons[1];
      else
        (*iter)[this->skill_cols.icon] = ImageStore::skillicons[3];
    }

    /* Set skill id, points and name. */
    (*iter)[this->skill_cols.id] = skill.id;
    (*iter)[this->skill_cols.skill] = &skills[i];
    (*iter)[this->skill_cols.points]
        = Helpers::get_dotted_str_from_int(skills[i].points);
    (*iter)[this->skill_cols.max_points]
        = Helpers::get_dotted_str_from_int(skills[i].points_max);

    (*iter)[this->skill_cols.name] = skill_name;
    (*iter)[this->skill_cols.level] = ImageStore::skill_progress
        (skills[i].level, skills[i].completed);
    (*iter)[this->skill_cols.primary] = ApiSkillTree::get_attrib_short_name(skills[i].details->primary);
    (*iter)[this->skill_cols.secondary] = ApiSkillTree::get_attrib_short_name(skills[i].details->secondary);

    /* Update group info. */
    iiter->second.sp += skills[i].points;
    iiter->second.empty = false;
  }

  /* Update the skillpoints for the groups. */
  for (IterMapType::iterator iter = iter_map.begin();
      iter != iter_map.end(); iter++)
  {
    /* Remove group with no skills. */
    if (iter->second.empty)
    {
      this->skill_store->erase(iter->second.iter);
      continue;
    }

    (*iter->second.iter)[this->skill_cols.points]
        = Helpers::get_dotted_str_from_int(iter->second.sp);

    (*iter->second.iter)[this->skill_cols.max_points]
        = Helpers::get_dotted_str_from_int(iter->second.max);

    /* Update of the SkillInTrainingInfo. */
    if (iter->first == skill_training.group)
      this->tree_group_iter = iter->second.iter;
  }
}

/* ---------------------------------------------------------------- */

void
GtkCharPage::request_documents (void)
{
  bool update_char = true;
  bool update_training = true;

  time_t evetime = EveTime::get_eve_time();
  std::string char_cached("<unknown>");
  std::string train_cached("<unknown>");

  /* Check which docs to re-request. */
  if (this->character->cs->valid && !this->character->cs->is_locally_cached())
  {
    time_t char_cached_t = this->character->cs->get_cached_until_t();
    char_cached = EveTime::get_gm_time_string(char_cached_t, false);
    if (evetime < char_cached_t)
      update_char = false;
  }

  if (this->character->sq->valid && !this->character->sq->is_locally_cached())
  {
    time_t train_cached_t = this->character->sq->get_cached_until_t();
    train_cached = EveTime::get_gm_time_string(train_cached_t, false);
    if (evetime < train_cached_t)
      update_training = false;
  }

  /* Message user if both docs are up-to-date. */
  if (!update_char && !update_training)
  {
    Gtk::MessageDialog md("Data is already up-to-date. Continue anyway?",
        false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO);
    md.set_secondary_text("The data you are about to refresh is already "
        "up-to-date.\n\n"
        "Character sheet expires at " + char_cached + "\n"
        "Skill queue sheet expires at " + train_cached + "\n\n"
        "You can continue and rerequest the data, but it will most "
        "likely don't change a thing.");
    md.set_title("Cache Status - GtkEveMon");
    md.set_transient_for(*this->parent_window);
    int result = md.run();

    switch (result)
    {
      default:
      case Gtk::RESPONSE_DELETE_EVENT:
      case Gtk::RESPONSE_NO:
        return;
      case Gtk::RESPONSE_YES:
        update_char = true;
        update_training = true;
        break;
    };
  }

  /* Request the documents. */
  if (update_char)
  {
    this->charsheet_info_label.set_text("Requesting...");
    this->character->request_charsheet();
  }

  if (update_training)
  {
    this->skillqueue_info_label.set_text("Requesting...");
    this->character->request_skillqueue();
  }
}

/* ---------------------------------------------------------------- */

bool
GtkCharPage::check_expired_sheets (void)
{
  /* Check if automatic update is enabled. */
  ConfValuePtr value = Config::conf.get_value("settings.auto_update_sheets");
  if (!value->get_bool())
    return true;

  ApiCharSheetPtr cs = this->character->cs;
  ApiSkillQueuePtr sq = this->character->sq;

  /* Skip automatic update if both sheets are cached. */
  if (sq->is_locally_cached() && cs->is_locally_cached())
    return true;

  //std::cout << "Checking for expired sheets..." << std::endl;

  time_t evetime = EveTime::get_eve_time();

  /* Check which docs to re-request. */
  if (!sq->valid || evetime >= sq->get_cached_until_t()
      || !cs->valid || evetime >= cs->get_cached_until_t())
    this->request_documents();

  return true;
}

/* ---------------------------------------------------------------- */

void
GtkCharPage::api_info_changed (void)
{
  /* Detect some API issues. */
  if (this->character->is_training() && this->character->training_skill == 0)
  {
    this->info_display.append(INFO_ERROR, "Skill in training is unknown!",
        "The EVE API reported a skill in training that is unknown to "
        "GtkEveMon. This typically happens if the data files are not "
        "up-to-date. If you already have recent data files, "
        "please report this issue! Thanks.\n\n"
        "Error: Skill with ID " + Helpers::get_string_from_int
        (this->character->training_info.skill_id) + " did not resolve!");
  }

  /* Update the char sheet and training sheet info. */
  this->update_cached_duration();
  this->update_charsheet_details();
  this->update_training_details();
  this->on_live_sp_value_update();
  this->on_live_sp_image_update();
}

/* ---------------------------------------------------------------- */

bool
GtkCharPage::update_cached_duration (void)
{
  time_t current = EveTime::get_eve_time();
  ApiCharSheetPtr cs = this->character->cs;
  ApiSkillQueuePtr sq = this->character->sq;

  if (sq->valid)
  {
    time_t cached_until = sq->get_cached_until_t();

    if (sq->is_locally_cached())
      this->skillqueue_info_label.set_text("Locally cached!");
    else if (cached_until > current)
      this->skillqueue_info_label.set_text(EveTime::get_minute_str_for_diff
          (cached_until - current) + " cached");
    else
      this->skillqueue_info_label.set_text("Ready for update!");
  }

  if (cs->valid)
  {
    time_t cached_until = cs->get_cached_until_t();

    if (cs->is_locally_cached())
      this->charsheet_info_label.set_text("Locally cached!");
    else if (cached_until > current)
      this->charsheet_info_label.set_text(EveTime::get_minute_str_for_diff
          (cached_until - current) + " cached");
    else
      this->charsheet_info_label.set_text("Ready for update!");
  }

  return true;
}

/* ---------------------------------------------------------------- */

void
GtkCharPage::create_tray_notify (void)
{
  this->tray_notify = Gtk::StatusIcon::create(ImageStore::skill);
  this->tray_notify->signal_activate().connect(sigc::mem_fun
     (*this, &GtkCharPage::remove_tray_notify));
  //this->tray_notify->set_blinking(true);
  this->tray_notify->set_tooltip(this->character->get_char_name() + " has "
      "completed " + this->training_label.get_text() + "!");
}

/* ---------------------------------------------------------------- */

void
GtkCharPage::remove_tray_notify (void)
{
  //this->tray_notify.reset(); // Compile errors for glibmm < 2.16
  this->tray_notify.clear(); // Deprecated
}

/* ---------------------------------------------------------------- */

void
GtkCharPage::popup_error_dialog (std::string const& title,
    std::string const& heading, std::string const& message)
{
  Gtk::MessageDialog md(heading, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);
  md.set_secondary_text(message);
  md.set_title(title + " - GtkEveMon");
  md.set_transient_for(*this->parent_window);
  md.run();
}

/* ---------------------------------------------------------------- */

void
GtkCharPage::exec_notification_handler (void)
{
  if (!this->character->valid_training_sheet())
    return;

  ConfValuePtr active = Config::conf.get_value("notifications.exec_handler");
  if (!active->get_bool())
    return;

  int ret;
  try
  {
    ret = Notifier::exec(this->character);
  }
  catch (Exception& e)
  {
    this->popup_error_dialog("Notification Error",
        "Problem executing notification handler", e);
    return;
  }

  if (ret != 0)
  {
    this->popup_error_dialog("Mailing Error",
        "Notification handler failed!",
        "The notification handler returned a non-zero exit code.");
    return;
  }
}

/* ---------------------------------------------------------------- */

void
GtkCharPage::on_skill_completed (void)
{
  /* Set up some GUI elements. */
  this->remaining_label.set_text("Completed!");
  this->finish_eve_label.set_text("---");
  this->finish_local_label.set_text("---");
  this->spph_label.set_text("0 SP per hour");
  this->spph_label.set_has_tooltip(false);
  this->live_sp_label.set_text("---");

  /* Update GUI to reflect changes. */
  this->update_charsheet_details();

  /* Now bring up some notifications. */
  ConfValuePtr show_popup = Config::conf.get_value
      ("notifications.show_popup_dialog");
  ConfValuePtr show_tray = Config::conf.get_value
      ("notifications.show_tray_icon");
  ConfValuePtr show_info = Config::conf.get_value
      ("notifications.show_info_bar");
  ConfValuePtr exec_handler = Config::conf.get_value
      ("notifications.exec_handler");

  if (show_tray->get_bool())
    this->create_tray_notify();

  if (show_info->get_bool())
    this->info_display.append(INFO_NOTIFICATION, "Skill training for <b>"
        + this->training_label.get_text() + "</b> completed!");

  if (exec_handler->get_bool())
    this->exec_notification_handler();

  if (show_popup->get_bool())
  {
    Gtk::MessageDialog* md = new Gtk::MessageDialog
        ("Skill training completed!",
        false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK);
    md->set_secondary_text("Congratulations. <b>"
        + this->character->get_char_name()
        + "</b> has just completed the skill training for <b>"
        + this->training_label.get_text() + "</b>.", true);
    md->set_title("Skill training completed!");
    md->set_transient_for(*this->parent_window);
    md->show_all();
    md->signal_response().connect(sigc::bind(sigc::mem_fun
        (*this, &GtkCharPage::delete_skill_completed_dialog), md));
  }
}

/* ---------------------------------------------------------------- */

void
GtkCharPage::on_close_clicked (void)
{
  Gtk::MessageDialog md("Really close the character?",
      false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_OK_CANCEL);
  md.set_secondary_text("You are about to remove <b>"
      + this->character->get_char_name() + "</b>\n"
      "from monitoring. Do you want to continue?", true);
  md.set_title("Close Character?");
  md.set_transient_for(*this->parent_window);

  int ret = md.run();
  if (ret != Gtk::RESPONSE_OK)
    return;

  CharacterList::request()->remove_character(this->character->get_char_id());
}

/* ---------------------------------------------------------------- */

bool
GtkCharPage::on_query_skillview_tooltip (int x, int y, bool /*key*/,
    Glib::RefPtr<Gtk::Tooltip> const& tooltip)
{
  Gtk::TreeModel::Path path;
  Gtk::TreeViewDropPosition pos;
  bool exists = this->skill_view.get_dest_row_at_pos(x, y, path, pos);
  if (!exists)
    return false;

  Gtk::TreeIter iter = this->skill_store->get_iter(path);
  int skill_id = (*iter)[this->skill_cols.id];
  ApiCharSheetSkill* cskill = (*iter)[this->skill_cols.skill];

  if (skill_id < 0 || cskill == 0)
    return false;

  this->skill_view.set_tooltip_row(tooltip, path);  /* Reposition tooltip. */
  GtkHelpers::create_tooltip(tooltip, cskill->details, cskill, this->character);
  return true;
}

/* ---------------------------------------------------------------- */

void
GtkCharPage::on_skill_activated (Gtk::TreeModel::Path const& path,
    Gtk::TreeViewColumn* /*col*/)
{
  Gtk::TreeIter iter = this->skill_store->get_iter(path);
  int skill_id = (*iter)[this->skill_cols.id];

  if (skill_id >= 0)
  {
    /* It's a skill. */
    GuiSkill* skillgui = new GuiSkill();
    skillgui->set_skill(skill_id);
  }
  else
  {
    /* It's probably a skill group. Expand/collapse. */
    if (this->skill_view.row_expanded(path))
      this->skill_view.collapse_row(path);
    else
      this->skill_view.expand_row(path, true);
  }
}

/* ---------------------------------------------------------------- */

bool
GtkCharPage::on_live_sp_value_update (void)
{
  /* Update the live values. This may trigger skill completed. */
  this->character->update_live_info();

  /* Check if the character is training. */
  if (!this->character->is_training())
    return true;

  /* A skill is in training. Fill some values. */
  this->remaining_label.set_text(this->character->get_remaining_text());
  this->live_sp_label.set_text(Helpers::get_dotted_str_from_uint
      (this->character->training_level_sp) + " SP ("
      + Helpers::get_string_from_double
      (this->character->training_level_done * 100.0, 2) + "%)");

  /* Check if the character sheet is valid. */
  if (!this->character->cs->valid)
    return true;

  /* Character sheet is also valid. Fill some more values. */
  this->skill_points_label.set_text(Helpers::get_dotted_str_from_uint
      (this->character->char_live_sp));

  /* Don't update character list if skill in training is unknown to char. */
  if (this->character->training_cskill == 0)
    return true;

  (*this->tree_skill_iter)[this->skill_cols.points] =
      Helpers::get_dotted_str_from_uint(this->character->training_skill_sp);
  (*this->tree_group_iter)[this->skill_cols.points] =
      Helpers::get_dotted_str_from_uint(this->character->char_group_live_sp);

  return true;
}

/* ---------------------------------------------------------------- */

bool
GtkCharPage::on_live_sp_image_update (void)
{
  if (!this->character->cs->valid || !this->character->is_training())
    return true;

  /* Don't update graphics if skill in training is unknown to char. */
  if (this->character->training_cskill == 0)
    return true;

  Glib::RefPtr<Gdk::Pixbuf> new_icon = ImageStore::skill_progress
      (this->character->training_cskill->level,
      this->character->training_level_done);
  (*this->tree_skill_iter)[this->skill_cols.level] = new_icon;

  return true;
}

/* ---------------------------------------------------------------- */

void
GtkCharPage::on_api_error (EveApiDocType dt, std::string msg, bool cached)
{
  std::string doc;
  switch (dt)
  {
    case API_DOCTYPE_CHARSHEET:
      doc = "CharacterSheet.xml";
      this->charsheet_info_label.set_text("Error requesting!");
      break;
    case API_DOCTYPE_SKILLQUEUE:
      doc = "SkillQueue.xml";
      this->skillqueue_info_label.set_text("Error requesting!");
      break;
    default:
      std::cout << "Warning: Received API error for unknown DT!" << std::endl;
      return;
  }

  std::cout << "Error: Failed to request " << doc << ": " << msg << std::endl;

  InfoItemType info_type;
  std::string heading;
  if (cached)
  {
    info_type = INFO_WARNING;
    heading = "Error requesting " + doc + "! Using cache.";
  }
  else
  {
    info_type = INFO_ERROR;
    heading = "Error requesting " + doc + "!";
  }

  this->info_display.append(info_type, heading,
      "There was an error while requesting " + doc + " from the EVE API. "
      "The EVE API is either offline, a network error occurred, or the "
      "requested document is not understood by GtkEveMon. "
      "The error message is:\n\n" + msg);
}

/* ---------------------------------------------------------------- */

void
GtkCharPage::on_skilltree_error (std::string const& e)
{
  std::cout << "Error requesting skill tree: " << e << std::endl;

  this->info_display.append(INFO_ERROR,
      "Error requesting skill tree!",
      "There was an error while parsing the skill tree. "
      "Reasons might be: The file was not found, the file "
      "is currupted, the file uses a new syntax unknown "
      "to GtkEveMon. The error message is:\n\n" + e);
}

/* ---------------------------------------------------------------- */

void
GtkCharPage::on_info_clicked (void)
{
  /* Enable this to rape the info button to send notifications. */
  //this->on_skill_completed();
  //return;

  Gtk::MessageDialog md("",
      false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK);
  md.set_title("Character Information - GtkEveMon");
  md.set_transient_for(*this->parent_window);

  if (!this->character->cs->valid)
  {
    md.set_message("<big><b>Character sheet is invalid!</b></big>", true);
    md.run();
    return;
  }

  std::stringstream ss;

  ss << "<big><b>Clone Information</b></big>\n\n";
  ss << "Last timed respec: " << this->character->cs->last_timed_respec << "\n";
  ss << "Last respec: " << this->character->cs->last_respec << "\n";
  ss << "Free respecs: " << this->character->cs->free_respecs << "\n";
  ss << "Free skill points: " << this->character->cs->free_sp << "\n";
  ss << "Last clone jump: " << this->character->cs->last_clone_jump << "\n";

  ss << "\n\n";
  ss << "<big><b>Sheet Cache Status</b></big>\n\n";
  ss << "Character sheet expires at " <<
      EveTime::get_gm_time_string(this->character->cs->get_cached_until_t(),
      false) << "\n";

  if (this->character->valid_training_sheet())
  {
    ss << "Skill queue sheet expires at "
        << EveTime::get_gm_time_string
        (this->character->sq->get_cached_until_t(), false);;
  }

  md.set_message(ss.str(), true);
  md.run();
}

/* ---------------------------------------------------------------- */

void
GtkCharPage::on_skillqueue_clicked (void)
{
  Gtk::Window* skillqueue = new GuiSkillQueue(this->character);
  skillqueue->set_transient_for(*this->parent_window);
}
