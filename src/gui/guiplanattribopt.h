// This file is part of GtkEveMon.
//
// GtkEveMon is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// You should have received a copy of the GNU General Public License
// along with GtkEveMon. If not, see <http://www.gnu.org/licenses/>.

#ifndef GUI_PLAN_ATTRIB_OPT_HEADER
#define	GUI_PLAN_ATTRIB_OPT_HEADER

#include <gtkmm.h>

#include "winbase.h"
#include "gtktrainingplan.h"

/* The minimum number of points that have to be assigned to each attribute. */
#define MINIMUM_VALUE_PER_ATTRIB 17
/* The maximum number of points that can be assigned to each attribute. */
#define MAXIMUM_VALUE_PER_ATTRIB 27

class GtkTreeModelColumnsOptimizer : public GtkTreeModelColumns
{
  public:
    Gtk::TreeModelColumn<Glib::ustring> difference;

    GtkTreeModelColumnsOptimizer(void);
};

/* ---------------------------------------------------------------- */

class GtkTreeViewColumnsOptimizer : public GtkTreeViewColumns
{
  public:
    Gtk::TreeView::Column difference;

    GtkTreeViewColumnsOptimizer(Gtk::TreeView* view,
        GtkTreeModelColumnsOptimizer* cols);
};

/* ---------------------------------------------------------------- */

class GuiPlanAttribOpt : public WinBase
{
  private:
    GtkSkillList plan;
    std::size_t plan_offset;

    Gtk::Notebook notebook;
    Gtk::RadioButton rb_whole_plan;
    Gtk::RadioButton rb_partial_plan;
    Gtk::ComboBoxText skill_selection;

    Gtk::Label base_cha_label;
    Gtk::Label base_intl_label;
    Gtk::Label base_per_label;
    Gtk::Label base_mem_label;
    Gtk::Label base_wil_label;

    Gtk::Label total_cha_label;
    Gtk::Label total_intl_label;
    Gtk::Label total_per_label;
    Gtk::Label total_mem_label;
    Gtk::Label total_wil_label;

    Gtk::Label original_time_label;
    Gtk::Label best_time_label;
    Gtk::Label difference_time_label;
    Gtk::Box warning_box;

    GtkTreeModelColumnsOptimizer cols;
    Glib::RefPtr<Gtk::ListStore> liststore;
    Gtk::TreeView treeview;
    GtkTreeViewColumnsOptimizer viewcols;

  private:
    Gtk::Widget* create_config_page (void);
    Gtk::Widget* create_attrib_page (void);
    Gtk::Widget* create_breakdown_page (void);

    void on_calculate_clicked (void);
    void set_selection_sensitivity (bool sensitive);
    void optimize_plan (void);

  public:
    GuiPlanAttribOpt (void);
    void set_plan (GtkSkillList const& plan);
};

#endif	/* GUI_PLAN_ATTRIB_OPT_HEADER */
