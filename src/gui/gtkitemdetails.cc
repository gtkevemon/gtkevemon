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
#include "imagestore.h"
#include "gtkhelpers.h"
#include "gtkdefines.h"
#include "gtkitemdetails.h"

GtkItemHistory::GtkItemHistory (void)
  : Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 0)
{
  this->history_pos = 0;

  this->back_but.set_image_from_icon_name("go-previous", Gtk::ICON_SIZE_MENU);
  this->next_but.set_image_from_icon_name("go-next", Gtk::ICON_SIZE_MENU);
  this->back_but.set_relief(Gtk::RELIEF_NONE);
  this->next_but.set_relief(Gtk::RELIEF_NONE);
  this->position_label.set_text("0/0");

  this->pack_start(this->back_but, false, false, 0);
  this->pack_start(this->position_label, false, false, 2);
  this->pack_start(this->next_but, false, false, 0);

  this->back_but.signal_clicked().connect(sigc::mem_fun
      (*this, &GtkItemHistory::back_clicked));
  this->next_but.signal_clicked().connect(sigc::mem_fun
      (*this, &GtkItemHistory::next_clicked));

  this->update_sensitive();
}

/* ---------------------------------------------------------------- */

void
GtkItemHistory::append_element (ApiElement const* elem)
{
  /* TODO: Search element, if found, erase it and push to end. */
  this->history.push_back(elem);

  while (this->history.size() > HISTORY_MAX_SIZE)
    this->history.erase(this->history.begin());

  this->history_pos = this->history.size() - 1;

  this->update_pos_label();
  this->update_sensitive();
  this->sig_elem_changed.emit(this->history[this->history_pos]);
}

/* ---------------------------------------------------------------- */

void
GtkItemHistory::update_sensitive (void)
{
  this->back_but.set_sensitive(this->history_pos > 0);
  this->next_but.set_sensitive(!this->history.empty()
      && this->history_pos < this->history.size() - 1);
}

/* ---------------------------------------------------------------- */

void
GtkItemHistory::update_pos_label (void)
{
  this->position_label.set_text(Helpers::get_string_from_sizet
      (this->history_pos + 1) + "/" + Helpers::get_string_from_sizet
      (this->history.size()));
}

/* ----------------------------------------------------------------- */

void
GtkItemHistory::back_clicked (void)
{
  if (this->history_pos > 0)
    this->history_pos -= 1;

  this->update_pos_label();
  this->update_sensitive();
  this->sig_elem_changed.emit(this->history[this->history_pos]);
}

/* ---------------------------------------------------------------- */

void
GtkItemHistory::next_clicked (void)
{
  if (this->history_pos < this->history.size() -1)
    this->history_pos += 1;

  this->update_pos_label();
  this->update_sensitive();
  this->sig_elem_changed.emit(this->history[this->history_pos]);
}

/* ================================================================ */

GtkDependencyList::GtkDependencyList (bool elem_indicator)
  : deps_store(Gtk::TreeStore::create(deps_cols)),
    deps_view(deps_store)
{
  Gtk::TreeViewColumn* deps_col_name = Gtk::manage(new Gtk::TreeViewColumn);
  deps_col_name->set_title("Name");
  deps_col_name->pack_start(this->deps_cols.icon, false);
  deps_col_name->pack_start(this->deps_cols.name, true);
  if (elem_indicator)
    this->deps_view.append_column("T", this->deps_cols.elem_icon);
  this->deps_view.append_column(*deps_col_name);
  this->deps_view.set_headers_visible(false);
  this->deps_view.get_selection()->set_mode(Gtk::SELECTION_SINGLE);
  //this->deps_view.set_show_expanders(false);
  this->deps_view.set_expander_column(*deps_col_name);
  this->deps_view.set_enable_tree_lines(true);

  this->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_ALWAYS);
  this->set_shadow_type(Gtk::SHADOW_ETCHED_IN);
  this->add(this->deps_view);

  this->deps_view.signal_row_activated().connect
      (sigc::mem_fun(*this, &GtkDependencyList::on_row_activated));
  this->deps_view.signal_button_press_myevent().connect(sigc::mem_fun
      (*this, &GtkDependencyList::on_view_button_pressed));
  this->deps_view.signal_query_tooltip().connect(sigc::mem_fun
      (*this, &GtkDependencyList::on_query_element_tooltip));
  this->deps_view.set_has_tooltip(true);
}

/* ---------------------------------------------------------------- */

void
GtkDependencyList::set_skill (ApiSkill const* skill)
{
  this->deps_store->clear();
  this->recurse_append_skill_req(skill, this->deps_store->append(), 1);
  this->deps_view.expand_all();
}

/* ---------------------------------------------------------------- */

void
GtkDependencyList::set_cert (ApiCert const* cert)
{
  this->deps_store->clear();
  this->recurse_append_cert_req(cert, this->deps_store->append());

  Gtk::TreePath path("0");
  this->deps_view.expand_row(path, false);
  //this->deps_view.expand_all();
}

/* ---------------------------------------------------------------- */

void
GtkDependencyList::recurse_append_skill_req (ApiSkill const* skill,
    Gtk::TreeModel::iterator slot, int level, bool recurse)
{
  (*slot)[this->deps_cols.name] = skill->name + " "
      + Helpers::get_roman_from_int(level);
  (*slot)[this->deps_cols.data] = skill;

  ApiCharSheetSkill* cskill = this->character->cs->get_skill_for_id(skill->id);
  Glib::RefPtr<Gdk::Pixbuf> skill_icon;
  if (cskill == 0)
    skill_icon = ImageStore::skilldeps[0];
  else if (cskill->level >= level)
    skill_icon = ImageStore::skilldeps[2];
  else
    skill_icon = ImageStore::skilldeps[1];
  (*slot)[this->deps_cols.icon] = skill_icon;
  (*slot)[this->deps_cols.elem_icon] = ImageStore::skillicons[1];

  if (!recurse)
    return;

  ApiSkillTreePtr tree = ApiSkillTree::request();
  for (unsigned int i = 0; i < skill->deps.size(); ++i)
  {
    int skill_id = skill->deps[i].first;
    int newlevel = skill->deps[i].second;

    // Prevent endless recursion
    bool recurse_this = true;
    if (skill_id == skill->id)
        recurse_this = false;

    ApiSkill const* newskill = tree->get_skill_for_id(skill_id);
    this->recurse_append_skill_req(newskill,
        this->deps_store->append(slot->children()), newlevel, recurse_this);
  }
}

/* ---------------------------------------------------------------- */

void
GtkDependencyList::recurse_append_cert_req (ApiCert const* cert,
    Gtk::TreeModel::iterator slot)
{
  (*slot)[this->deps_cols.name] = cert->class_details->name + " ("
      + Glib::ustring(ApiCertTree::get_name_for_grade(cert->grade)) + ")";
  (*slot)[this->deps_cols.data] = cert;

  Glib::RefPtr<Gdk::Pixbuf> cert_icon;
  int grade = this->character->cs->get_grade_for_class(cert->class_details->id);
  if (grade == 0)
    cert_icon = ImageStore::skilldeps[0];
  else if (grade >= cert->grade)
    cert_icon = ImageStore::skilldeps[2];
  else
    cert_icon = ImageStore::skilldeps[1];

  (*slot)[this->deps_cols.icon] = cert_icon;
  (*slot)[this->deps_cols.elem_icon] = ImageStore::certificate_small;

  ApiSkillTreePtr stree = ApiSkillTree::request();
  ApiCertTreePtr ctree = ApiCertTree::request();

  for (unsigned int i = 0; i < cert->certdeps.size(); ++i)
  {
    int cert_id = cert->certdeps[i].first;
    ApiCert const* newcert = ctree->get_certificate_for_id(cert_id);
    this->recurse_append_cert_req(newcert,
        this->deps_store->append(slot->children()));
  }

  for (unsigned int i = 0; i < cert->skilldeps.size(); ++i)
  {
    int skill_id = cert->skilldeps[i].first;
    int newlevel = cert->skilldeps[i].second;
    ApiSkill const* newskill = stree->get_skill_for_id(skill_id);
    this->recurse_append_skill_req(newskill,
        this->deps_store->append(slot->children()), newlevel);
  }
}

/* ---------------------------------------------------------------- */

void
GtkDependencyList::on_row_activated (Gtk::TreeModel::Path const& path,
    Gtk::TreeViewColumn* /*col*/)
{
  Gtk::TreeModel::iterator iter = this->deps_store->get_iter(path);
  ApiElement const* elem = (*iter)[this->deps_cols.data];

  if (elem == 0)
    return;

  this->sig_element_selected.emit(elem);
}

/* ---------------------------------------------------------------- */

void
GtkDependencyList::on_view_button_pressed (GdkEventButton* event)
{
  if (event->type != GDK_BUTTON_PRESS || event->button != 3)
    return;

  Glib::RefPtr<Gtk::TreeView::Selection> selection
      = this->deps_view.get_selection();

  if (selection->count_selected_rows() != 1)
    return;

  Gtk::TreeModel::iterator iter = selection->get_selected();
  ApiElement const* elem = (*iter)[this->deps_cols.data];
  if (elem == 0) // || elem->get_type() != API_ELEM_SKILL)
    return;

  switch (elem->get_type())
  {
    case API_ELEM_SKILL:
    {
      ApiSkill const* skill = (ApiSkill const*)elem;
      GtkSkillContextMenu* context = Gtk::manage(new GtkSkillContextMenu);
      context->set_skill(skill,
          this->character->cs->get_level_for_skill(skill->id));
      context->popup(event->button, event->time);
      context->signal_planning_requested().connect(sigc::mem_fun
          (this->sig_planning_requested, &SignalPlanningRequested::emit));
      break;
    }

    case API_ELEM_CERT:
    {
      ApiCert const* cert = (ApiCert const*)elem;
      GtkCertContextMenu* context = Gtk::manage(new GtkCertContextMenu);
      context->set_cert(cert);
      context->popup(event->button, event->time);
      context->signal_planning_requested().connect(sigc::mem_fun
          (this->sig_planning_requested, &SignalPlanningRequested::emit));
      break;
    }

    default:
      break;
  }

  // TODO make switch, allow certs
  // - certs deps to a new plan
  // - certs deps to the current plan (sensitive)
  // - implement: add a cert (signal cert planning requested?)


  return;
}

/* ---------------------------------------------------------------- */

bool
GtkDependencyList::on_query_element_tooltip (int x, int y, bool /*key*/,
    Glib::RefPtr<Gtk::Tooltip> const& tooltip)
{
  Gtk::TreeModel::Path path;
  Gtk::TreeViewDropPosition pos;

  bool exists = this->deps_view.get_dest_row_at_pos(x, y, path, pos);

  if (!exists)
    return false;

  Gtk::TreeIter iter = this->deps_store->get_iter(path);
  ApiElement const* elem = (*iter)[this->deps_cols.data];

  if (elem == 0)
    return false;

  this->deps_view.set_tooltip_row(tooltip, path); /* Reposition tooltip. */
  switch (elem->get_type())
  {
    case API_ELEM_SKILL:
      GtkHelpers::create_tooltip(tooltip, (ApiSkill const*)elem);
      break;
    case API_ELEM_CERT:
      GtkHelpers::create_tooltip(tooltip, (ApiCert const*)elem);
      break;
    default:
      return false;
  }

  return true;
}

/* ================================================================ */

GtkItemDetails::GtkItemDetails (void)
  : Gtk::Box(Gtk::ORIENTATION_VERTICAL, 5)
{
  this->element = 0;
  this->element_name.set_halign(Gtk::ALIGN_START);
  this->element_path.set_halign(Gtk::ALIGN_START);
  this->element_path.set_text("No item is selected.");

  Gtk::Table* elem_info_table = MK_TABLE(2, 3);
  elem_info_table->set_col_spacings(5);
  elem_info_table->attach(this->element_icon, 0, 1, 0, 2,
      Gtk::SHRINK, Gtk::FILL);
  elem_info_table->attach(this->element_path, 1, 2, 0, 1,
      Gtk::EXPAND | Gtk::FILL, Gtk::FILL);
  elem_info_table->attach(this->element_name, 1, 2, 1, 2,
      Gtk::EXPAND | Gtk::FILL, Gtk::FILL);
  elem_info_table->attach(this->history, 2, 3, 0, 2,
      Gtk::SHRINK, Gtk::FILL);

  this->set_border_width(5);
  this->pack_start(*elem_info_table, false, false, 0);
  this->pack_start(*MK_HSEP, false, false, 0);
  this->pack_start(this->details_box, true, true, 0);

  this->history.signal_elem_changed().connect(sigc::mem_fun
      (*this, &GtkItemDetails::on_element_changed));
  this->skill_details.signal_element_selected().connect
      (sigc::mem_fun(*this, &GtkItemDetails::set_element));
  this->skill_details.signal_planning_requested().connect(sigc::mem_fun
      (this->sig_planning_requested, &SignalPlanningRequested::emit));
  this->cert_details.signal_element_selected().connect
      (sigc::mem_fun(*this, &GtkItemDetails::set_element));
  this->cert_details.signal_planning_requested().connect(sigc::mem_fun
      (this->sig_planning_requested, &SignalPlanningRequested::emit));
}

/* ---------------------------------------------------------------- */

void
GtkItemDetails::set_element (ApiElement const* elem)
{
  if (elem == this->element)
    return;

  this->history.append_element(elem);
}

/* ---------------------------------------------------------------- */

void
GtkItemDetails::on_element_changed (ApiElement const* elem)
{
  if (this->details_box.get_children().empty())
  {
    this->details_box.pack_start(this->skill_details, true, true, 0);
    this->details_box.pack_start(this->cert_details, true, true, 0);
  }

  switch (elem->get_type())
  {
    case API_ELEM_SKILL:
    {
      ApiSkill const* skill = (ApiSkill const*)elem;
      ApiSkillTreePtr tree = ApiSkillTree::request();
      ApiSkillGroup const* group = tree->get_group_for_id(skill->group);
      this->element_path.set_markup("<b>" + group->name + "</b>");
      this->element_name.set_markup("<b>" + skill->name + " (rank "
          + Helpers::get_string_from_int(skill->rank) + ")</b>");
      this->element_icon.set(ImageStore::skill->scale_simple
          (35, 35, Gdk::INTERP_BILINEAR));

      this->skill_details.set_character(this->character);
      this->skill_details.set_skill(skill);

      this->cert_details.hide();
      this->skill_details.show_all();

      break;
    }

    case API_ELEM_CERT:
    {
      ApiCert const* cert = (ApiCert const*)elem;
      ApiCertClass const* cclass = cert->class_details;
      ApiCertCategory const* ccat = cclass->cat_details;
      this->element_path.set_markup("<b>" + ccat->name + " ("
          + ApiCertTree::get_name_for_grade(cert->grade) + ")</b>");
      this->element_name.set_markup("<b>" + cclass->name + "</b>");
      this->element_icon.set(ImageStore::certificate->scale_simple
          (35, 35, Gdk::INTERP_BILINEAR));

      this->cert_details.set_character(this->character);
      this->cert_details.set_certificate(cert);

      this->skill_details.hide();
      this->cert_details.show_all();

      break;
    }

    default:
      break;
  }

  this->element = elem;
}

/* ================================================================ */

GtkSkillDetails::GtkSkillDetails (void)
  : Gtk::Box(Gtk::ORIENTATION_VERTICAL, 5),
    desc_buffer(Gtk::TextBuffer::create()),
    deps(false)
{
  this->skill_primary.set_halign(Gtk::ALIGN_START);
  this->skill_secondary.set_halign(Gtk::ALIGN_START);
  for (unsigned int i = 0; i < 5; ++i)
  {
    this->skill_level[i].set_halign(Gtk::ALIGN_START);
    this->skill_level[i].set_valign(Gtk::ALIGN_START);
    this->skill_level[i].set_selectable(true);
  }

  Gtk::TextView* text_view = Gtk::manage(new Gtk::TextView(this->desc_buffer));
  text_view->set_editable(false);
  text_view->set_left_margin(3);
  text_view->set_wrap_mode(Gtk::WRAP_WORD);
  Gtk::ScrolledWindow* text_scwin = MK_SCWIN;
  text_scwin->set_shadow_type(Gtk::SHADOW_ETCHED_IN);
  text_scwin->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_ALWAYS);
  text_scwin->add(*text_view);

  Gtk::Label* primary_label = MK_LABEL("Primary:");
  Gtk::Label* secondary_label = MK_LABEL("Secondary:");
  primary_label->set_halign(Gtk::ALIGN_START);
  secondary_label->set_halign(Gtk::ALIGN_START);
  Gtk::Label* to_level_label[5];
  for (unsigned int i = 0; i < 5; ++i)
  {
    to_level_label[i] = Gtk::manage(new Gtk::Label);
    to_level_label[i]->set_text("To level "
        + Helpers::get_string_from_int(i + 1) + ":");
    to_level_label[i]->set_halign(Gtk::ALIGN_START);
    to_level_label[i]->set_valign(Gtk::ALIGN_START);
  }

  Gtk::Table* details_table = Gtk::manage(new Gtk::Table(7, 3, false));
  details_table->set_col_spacings(5);
  details_table->attach(*primary_label, 0, 1, 0, 1,
      Gtk::FILL | Gtk::SHRINK, Gtk::SHRINK);
  details_table->attach(*secondary_label, 0, 1, 1, 2,
      Gtk::FILL | Gtk::SHRINK, Gtk::SHRINK);
  details_table->attach(*to_level_label[0], 0, 1, 2, 3,
      Gtk::FILL | Gtk::SHRINK, Gtk::SHRINK);
  details_table->attach(*to_level_label[1], 0, 1, 3, 4,
      Gtk::FILL | Gtk::SHRINK, Gtk::SHRINK);
  details_table->attach(*to_level_label[2], 0, 1, 4, 5,
      Gtk::FILL | Gtk::SHRINK, Gtk::SHRINK);
  details_table->attach(*to_level_label[3], 0, 1, 5, 6,
      Gtk::FILL | Gtk::SHRINK, Gtk::SHRINK);
  details_table->attach(*to_level_label[4], 0, 1, 6, 7,
      Gtk::FILL | Gtk::SHRINK, Gtk::EXPAND | Gtk::FILL);

  details_table->attach(this->skill_primary, 1, 2, 0, 1,
      Gtk::FILL | Gtk::SHRINK, Gtk::SHRINK);
  details_table->attach(this->skill_secondary, 1, 2, 1, 2,
      Gtk::FILL | Gtk::SHRINK, Gtk::SHRINK);
  details_table->attach(this->skill_level[0], 1, 2, 2, 3,
      Gtk::FILL | Gtk::SHRINK, Gtk::SHRINK);
  details_table->attach(this->skill_level[1], 1, 2, 3, 4,
      Gtk::FILL | Gtk::SHRINK, Gtk::SHRINK);
  details_table->attach(this->skill_level[2], 1, 2, 4, 5,
      Gtk::FILL | Gtk::SHRINK, Gtk::SHRINK);
  details_table->attach(this->skill_level[3], 1, 2, 5, 6,
      Gtk::FILL | Gtk::SHRINK, Gtk::SHRINK);
  details_table->attach(this->skill_level[4], 1, 2, 6, 7,
      Gtk::FILL | Gtk::SHRINK, Gtk::EXPAND | Gtk::FILL);

  details_table->attach(*text_scwin, 2, 3, 0, 7,
      Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::EXPAND);

  /* Pack stuff together, create vpaned window. */
  Gtk::VPaned* vpaned = Gtk::manage(new Gtk::VPaned);
  vpaned->set_position(1);
  vpaned->pack1(*details_table, true, false);
  vpaned->pack2(this->deps, true, false);

  this->pack_start(*vpaned, true, true, 0);

  /* Init some labels. */
  this->skill_primary.set_text("<not available>");
  this->skill_secondary.set_text("<not available>");
  for (unsigned int i = 0; i < 5; ++i)
    this->skill_level[i].set_text("0d 0h");
  //this->desc_buffer->set_text("");

  this->deps.signal_planning_requested().connect(sigc::mem_fun
      (this->sig_planning_requested, &SignalPlanningRequested::emit));
  this->deps.signal_element_selected().connect(sigc::mem_fun
      (this->sig_element_selected, &SignalApiElementSelected::emit));
}

/* ---------------------------------------------------------------- */

void
GtkSkillDetails::set_skill (ApiSkill const* skill)
{
  /* Fill character invariant values. */
  this->skill_primary.set_text
      (ApiSkillTree::get_attrib_name(skill->primary));
  this->skill_secondary.set_text
      (ApiSkillTree::get_attrib_name(skill->secondary));
  this->desc_buffer->set_text(skill->desc);

  ApiCharSheetPtr cs = this->character->cs;

  /* Fill character related values. */
  if (!cs->valid)
    return;

  ApiCharSheetSkill* cskill = cs->get_skill_for_id(skill->id);
  time_t timediff = 0;
  double spps = (double)cs->get_spph_for_skill(skill) / 3600.0;

  for (unsigned int i = 0; i < 5; ++i)
  {
    Glib::ustring to_level_str;
    if (cskill == 0 || cskill->level < (int)i)
    {
      int start_sp = cs->calc_start_sp(i, skill->rank);
      int dest_sp = cs->calc_dest_sp(i, skill->rank);
      timediff += (time_t)((double)(dest_sp - start_sp) / spps);
      to_level_str = EveTime::get_string_for_timediff(timediff, true);
    }
    else if (cskill->level == (int)i)
    {
      int diff_sp = cskill->points_dest - cskill->points;
      timediff += (time_t)((double)diff_sp / spps);
      to_level_str = EveTime::get_string_for_timediff(timediff, true);
      if (cskill->completed != 0.0)
        to_level_str += " (" + Helpers::get_string_from_double
            (cskill->completed * 100.0, 0) + "%)";
    }
    else
    {
      to_level_str = "---";
    }

    this->skill_level[i].set_text(to_level_str);
  }

  this->deps.set_character(this->character);
  this->deps.set_skill(skill);
}

/* ================================================================ */

GtkCertDetails::GtkCertDetails (void)
  : desc_buffer(Gtk::TextBuffer::create()),
    deps(true)
{
  Gtk::TextView* text_view = Gtk::manage(new Gtk::TextView(this->desc_buffer));
  text_view->set_editable(false);
  text_view->set_left_margin(3);
  text_view->set_wrap_mode(Gtk::WRAP_WORD);
  Gtk::ScrolledWindow* text_scwin = MK_SCWIN;
  text_scwin->set_shadow_type(Gtk::SHADOW_ETCHED_IN);
  text_scwin->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_ALWAYS);
  text_scwin->add(*text_view);
  text_scwin->set_size_request(-1, 100);

  /* Pack stuff together, create vpaned window. */
  Gtk::VPaned* vpaned = Gtk::manage(new Gtk::VPaned);
  vpaned->set_position(1);
  vpaned->pack1(*text_scwin, true, false);
  vpaned->pack2(this->deps, true, false);

  this->pack_start(*vpaned, true, true, 0);

  this->deps.signal_planning_requested().connect(sigc::mem_fun
      (this->sig_planning_requested, &SignalPlanningRequested::emit));
  this->deps.signal_element_selected().connect(sigc::mem_fun
      (this->sig_element_selected, &SignalApiElementSelected::emit));
}

/* ---------------------------------------------------------------- */

void
GtkCertDetails::set_certificate (ApiCert const* cert)
{
  this->desc_buffer->set_text(cert->desc);

  this->deps.set_character(this->character);
  this->deps.set_cert(cert);
}
