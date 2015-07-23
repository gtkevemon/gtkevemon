// This file is part of GtkEveMon.
//
// GtkEveMon is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// You should have received a copy of the GNU General Public License
// along with GtkEveMon. If not, see <http://www.gnu.org/licenses/>.

#ifndef GTK_ITEM_DEATILS_HEADER
#define GTK_ITEM_DEATILS_HEADER

#include <gtkmm.h>

#include "api/apiskilltree.h"
#include "api/apicerttree.h"
#include "bits/character.h"
#include "gtkplannerbase.h"

/* The maximum history size. */
#define HISTORY_MAX_SIZE 9

/*
 * This class keeps a history of recently viewed API elements (these
 * are skills, certificates and items). New elements need to be added
 * with append_element. A signal is then fired to notify the GUI
 * to update to the new element.
 */
class GtkItemHistory : public Gtk::Box
{
  private:
    std::vector<ApiElement const*> history;
    std::size_t history_pos;
    SignalApiElementSelected sig_elem_changed;

    Gtk::Button back_but;
    Gtk::Button next_but;
    Gtk::Label position_label;

    void update_sensitive (void);
    void update_pos_label (void);
    void back_clicked (void);
    void next_clicked (void);

  public:
    GtkItemHistory (void);

    void append_element (ApiElement const* elem);
    SignalApiElementSelected& signal_elem_changed (void);
};

/* ---------------------------------------------------------------- */

/*
 * A base class for various GUI elements that need the character sheet
 * and fire two important events: Selection of new API elements and
 * requests for enqueing a skill to the training plan.
 */
class GtkItemDetailsBase
{
  protected:
    CharacterPtr character;
    SignalPlanningRequested sig_planning_requested;
    SignalApiElementSelected sig_element_selected;

  public:
    void set_character (CharacterPtr character);
    SignalPlanningRequested& signal_planning_requested (void);
    SignalApiElementSelected& signal_element_selected (void);
};

/* ---------------------------------------------------------------- */

/*
 * Creates a list of dependencies (skills or certificates) for the
 * specified element. The list contains both, skills and certificates
 * if the subject element is a certificate.
 */
class GtkDependencyList : public GtkItemDetailsBase, public Gtk::ScrolledWindow
{
  private:
    class GtkDependencyCols : public GuiPlannerElemCols
    {
      public:
        Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > elem_icon;
        GtkDependencyCols (void)
        { this->add(elem_icon); }
    } deps_cols;
    Glib::RefPtr<Gtk::TreeStore> deps_store;
    GtkListViewHelper deps_view;

  protected:
    void recurse_append_skill_req (ApiSkill const* skill,
        Gtk::TreeModel::iterator slot, int level, bool recurse = true);
    void recurse_append_cert_req (ApiCert const* cert,
        Gtk::TreeModel::iterator slot);
    void on_row_activated (Gtk::TreeModel::Path const& path,
        Gtk::TreeViewColumn* col);
    void on_view_button_pressed (GdkEventButton* event);
    bool on_query_element_tooltip (int x, int y, bool key,
        Glib::RefPtr<Gtk::Tooltip> const& tooltip);

  public:
    GtkDependencyList (bool elem_indicator);
    void set_skill (ApiSkill const* skill);
    void set_cert (ApiCert const* cert);
};

/* ---------------------------------------------------------------- */

/* This GUI class shows details for skills. */
class GtkSkillDetails : public GtkItemDetailsBase, public Gtk::Box
{
  private:
    /* Skill details. */
    Gtk::Label skill_primary;
    Gtk::Label skill_secondary;
    Gtk::Label skill_level[5];
    Glib::RefPtr<Gtk::TextBuffer> desc_buffer;
    GtkDependencyList deps;

  public:
    GtkSkillDetails (void);
    void set_skill (ApiSkill const* skill);
};

/* ---------------------------------------------------------------- */

/* This GUI class shows details for certificates. */
class GtkCertDetails : public GtkItemDetailsBase, public Gtk::Box
{
  private:
    Glib::RefPtr<Gtk::TextBuffer> desc_buffer;
    GtkDependencyList deps;

  public:
    GtkCertDetails (void);
    void set_certificate (ApiCert const* cert);
};

/* ---------------------------------------------------------------- */

/*
 * This GUI class is a container for detail GUIs. It shows the
 * icon and name of the item and displays the appropriate detail GUI.
 */
class GtkItemDetails : public GtkItemDetailsBase, public Gtk::Box
{
  private:
    ApiElement const* element;
    Gtk::Image element_icon;
    Gtk::Label element_path;
    Gtk::Label element_name;
    GtkItemHistory history;
    Gtk::Box details_box;
    GtkSkillDetails skill_details;
    GtkCertDetails cert_details;

  protected:
    void on_element_changed (ApiElement const* elem);

  public:
    GtkItemDetails (void);

    void set_element (ApiElement const* elem);
};

/* ---------------------------------------------------------------- */

inline SignalApiElementSelected&
GtkItemHistory::signal_elem_changed (void)
{
  return this->sig_elem_changed;
}

inline void
GtkItemDetailsBase::set_character (CharacterPtr character)
{
  this->character = character;
}

inline SignalPlanningRequested&
GtkItemDetailsBase::signal_planning_requested (void)
{
  return this->sig_planning_requested;
}

inline SignalApiElementSelected&
GtkItemDetailsBase::signal_element_selected (void)
{
  return this->sig_element_selected;
}

#endif /* GTK_ITEM_DEATILS_HEADER */
