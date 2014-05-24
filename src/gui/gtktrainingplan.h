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

#ifndef GTK_TRAINING_PLAN
#define GTK_TRAINING_PLAN

#include <gtkmm/button.h>
#include <gtkmm/label.h>
#include <gtkmm/box.h>
#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>

#include "bits/config.h"
#include "bits/character.h"
#include "gtkportrait.h"
#include "gtkcolumnsbase.h"
#include "gtkconfwidgets.h"

/* Update the time values for skills this milli seconds. */
#define PLANNER_SKILL_TIME_UPDATE 10000

enum GtkSkillIcon
{
  SKILL_STATUS_TRAINED,
  SKILL_STATUS_TRAINING,
  SKILL_STATUS_TRAINABLE,
  SKILL_STATUS_UNTRAINABLE,
  SKILL_STATUS_MISSING_DEPS
};

/* ---------------------------------------------------------------- */

struct GtkSkillInfo
{
  ApiSkill const* skill;
  bool is_objective;
  int plan_level;
  std::string user_notes;

  int start_sp;
  int dest_sp;
  time_t train_duration;
  time_t skill_duration;
  time_t finish_time;
  time_t start_time;
  double completed;
  int spph;
  GtkSkillIcon skill_icon;
};

/* ---------------------------------------------------------------- */

struct OptimalData
{
  time_t optimal_time;
  double spph;
  double intelligence;
  double memory;
  double perception;
  double willpower;
  double charisma;
};

class GtkSkillList : public std::vector<GtkSkillInfo>
{
  private:
    CharacterPtr character;
    unsigned int total_plan_sp;

  protected:
    void append_skill (ApiSkill const* skill, int level, bool objective);

  public:
    GtkSkillList (void);

    void set_character (CharacterPtr character);
    CharacterPtr get_character (void) const;

    void append_skill (ApiSkill const* skill, int level);
    void append_cert (ApiCert const* cert);

    void move_skill (unsigned int from, unsigned int to);
    //void fix_skill (unsigned int index);
    void insert_skill (unsigned int pos, GtkSkillInfo const& info);
    void release_skill (unsigned int index);
    void delete_skill (unsigned int index);
    void cleanup_skills (void);
    bool has_plan_dep_skills (unsigned int index);
    bool has_char_dep_skills (ApiSkill const* skill, int level);
    bool has_char_skill (ApiSkill const* skill, int level);
    bool has_plan_skill (ApiSkill const* skill, int level,
        bool make_objective = false);
    bool is_dependency (unsigned int index);

    /* Returns the total SP in the plan. */
    unsigned int get_total_plan_sp (void) const;

    /* Calculate all details for the skill plan. If attributes and
     * the learning level are specified, these are used instead
     * of the character ones. */
    void calc_details (void);
    void calc_details (ApiCharAttribs& attribs);
    //void simulate_select (unsigned int index);

    OptimalData get_optimal_data (void) const;

    double get_spph(void) const;
};

/* ---------------------------------------------------------------- */

class GtkTreeModelColumns : public Gtk::TreeModel::ColumnRecord
{
  public:
    Gtk::TreeModelColumn<ApiElement const*> skill;
    Gtk::TreeModelColumn<unsigned int> skill_index;
    Gtk::TreeModelColumn<bool> objective;
    Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > skill_icon;
    Gtk::TreeModelColumn<Glib::ustring> skill_name;
    Gtk::TreeModelColumn<Glib::ustring> train_duration;
    Gtk::TreeModelColumn<Glib::ustring> skill_duration;
    Gtk::TreeModelColumn<Glib::ustring> completed;
    Gtk::TreeModelColumn<Glib::ustring> attributes;
    Gtk::TreeModelColumn<Glib::ustring> est_start;
    Gtk::TreeModelColumn<Glib::ustring> est_finish;
    Gtk::TreeModelColumn<Glib::ustring> user_notes;
    Gtk::TreeModelColumn<int> spph;

  public:
    GtkTreeModelColumns (void);
};

/* ---------------------------------------------------------------- */

class GtkTreeViewColumns : public GtkColumnsBase
{
  public:
    typedef Glib::SignalProxy2<void, const Glib::ustring&,
        const Glib::ustring&> CellEditedSignal;
    typedef Glib::SignalProxy2<void, Gtk::CellEditable*,
        const Glib::ustring&> CellStartEditingSignal;
    typedef Glib::SignalProxy0<void> CellEditCanceledSignal;

  public:
    Gtk::TreeView::Column objective;
    Gtk::TreeView::Column skill_name;
    Gtk::TreeView::Column train_duration;
    Gtk::TreeView::Column skill_duration;
    Gtk::TreeView::Column completed;
    Gtk::TreeView::Column attributes;
    Gtk::TreeView::Column est_start;
    Gtk::TreeView::Column est_finish;
    Gtk::TreeView::Column user_notes;
    Gtk::TreeView::Column spph;

  public:
    GtkTreeViewColumns (Gtk::TreeView* view, GtkTreeModelColumns* cols);
    CellEditedSignal signal_user_notes_changed (void);
    CellStartEditingSignal signal_editing_started (void);
    CellEditCanceledSignal signal_editing_canceled (void);
};

/* ---------------------------------------------------------------- */

class GtkTrainingPlan : public Gtk::VBox
{
  private:
    CharacterPtr character;
    GtkSkillList skills;

    GtkConfSectionSelection plan_selection;
    ConfSectionPtr plan_section;

    GtkPortrait portrait;
    Gtk::Button delete_plan_but;
    Gtk::Button create_plan_but;
    Gtk::Button rename_plan_but;
    Gtk::Button clean_plan_but;
    Gtk::Button column_conf_but;
    Gtk::Button export_plan_but;
    Gtk::Button import_plan_but;
    Gtk::Button optimize_att_but;
    Gtk::Label total_time;
    Gtk::Label optimal_time;

    GtkTreeModelColumns cols;
    Glib::RefPtr<Gtk::ListStore> liststore;
    Gtk::TreeView treeview;
    GtkTreeViewColumns viewcols;
    bool updating_liststore;
    int reorder_new_index;
    int currently_editing;

    sigc::signal<void, ApiSkill const*> sig_skill_activated;

  protected:
    void update_plan (bool rebuild);

    void init_from_config (void);
    void store_to_config (void);
    void skill_plan_changed (ConfSectionPtr section);
    void save_current_plan (void);
    void load_current_plan (void);
    void on_create_skill_plan (void);
    void on_delete_skill_plan (void);
    void on_rename_skill_plan (void);
    void on_cleanup_skill_plan (void);
    void on_objective_toggled (Glib::ustring const& path);
    void on_user_notes_edited (Glib::ustring const& path,
        Glib::ustring const& value);
    void on_user_notes_editing (Gtk::CellEditable* editable,
        const Glib::ustring& path);
    void on_user_notes_editing_canceled (void);
    void on_export_plan (void);
    void on_import_plan (void);
    void on_optimize_att (void);

    void on_row_inserted (Gtk::TreePath const& path,
        Gtk::TreeModel::iterator const& iter);
    void on_row_deleted (Gtk::TreePath const& path);
    void on_row_activated (Gtk::TreeModel::Path const& path,
        Gtk::TreeViewColumn* column);
    bool on_update_skill_time (void);
    bool on_query_skillview_tooltip (int x, int y, bool key,
        Glib::RefPtr<Gtk::Tooltip> const& tooltip);

  public:
    GtkTrainingPlan (void);
    ~GtkTrainingPlan (void);

    void set_character (CharacterPtr character);

    void append_element (ApiElement const* skill, int level);

    sigc::signal<void, ApiSkill const*>& signal_skill_activated (void);
};

/* ---------------------------------------------------------------- */

inline void
GtkSkillList::set_character (CharacterPtr character)
{
  this->character = character;
}

inline CharacterPtr
GtkSkillList::get_character (void) const
{
  return this->character;
}

inline void
GtkSkillList::append_skill (ApiSkill const* skill, int level)
{
  this->append_skill(skill, level, true);
}

inline unsigned int
GtkSkillList::get_total_plan_sp (void) const
{
  return this->total_plan_sp;
}

inline GtkTreeViewColumns::CellEditedSignal
GtkTreeViewColumns::signal_user_notes_changed (void)
{
  return ((Gtk::CellRendererText*)this->user_notes
      .get_first_cell_renderer())->signal_edited();
}

inline GtkTreeViewColumns::CellStartEditingSignal
GtkTreeViewColumns::signal_editing_started (void)
{
  return this->user_notes.get_first_cell_renderer()->signal_editing_started();
}

inline GtkTreeViewColumns::CellEditCanceledSignal
GtkTreeViewColumns::signal_editing_canceled (void)
{
  return this->user_notes.get_first_cell_renderer()->signal_editing_canceled();
}

inline sigc::signal<void, ApiSkill const*>&
GtkTrainingPlan::signal_skill_activated (void)
{
  return this->sig_skill_activated;
}

#endif /* GTK_TRAINING_PLAN */
