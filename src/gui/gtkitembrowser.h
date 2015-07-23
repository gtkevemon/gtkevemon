// This file is part of GtkEveMon.
//
// GtkEveMon is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// You should have received a copy of the GNU General Public License
// along with GtkEveMon. If not, see <http://www.gnu.org/licenses/>.

#ifndef GTK_ITEM_BROWSER_HEADER
#define GTK_ITEM_BROWSER_HEADER

#include <gtkmm.h>

#include "api/apicharsheet.h"
#include "gtkplannerbase.h"

class ItemBrowserBase
{
  private:
    SignalApiElementSelected sig_element_selected;
    SignalApiElementActivated sig_element_activated;
    SignalPlanningRequested sig_planning_requested;

  protected:
    ApiCharSheetPtr charsheet;
    GuiPlannerElemCols cols;
    Glib::RefPtr<Gtk::TreeStore> store;
    GtkListViewHelper view;

  protected:
    /* Executed if the item selection changed. */
    void on_selection_changed (void);
    /* Executed if some item is double clicked. */
    void on_row_activated (Gtk::TreeModel::Path const& path,
        Gtk::TreeViewColumn* col);
    /* Executed if a mouse button is pressed. */
    void on_view_button_pressed (GdkEventButton* event);
    /* Executed if planning was requested from the context menu. */
    void on_planning_requested (ApiElement const* skill, int level);
    /* Tooltip query. */
    bool on_query_element_tooltip (int x, int y, bool key,
        Glib::RefPtr<Gtk::Tooltip> const& tooltip);

  public:
    ItemBrowserBase (void);
    virtual ~ItemBrowserBase (void) {};
    virtual void set_character (ApiCharSheetPtr charsheet);
    virtual void fill_store (void) = 0;

    SignalApiElementSelected& signal_element_selected (void);
    SignalApiElementActivated& signal_element_activated (void);
    SignalPlanningRequested& signal_planning_requested (void);
};

/* ---------------------------------------------------------------- */

class GtkSkillBrowser : public ItemBrowserBase, public Gtk::Box
{
  private:
    Gtk::Entry filter_entry;
    Gtk::ComboBoxText filter_cb;
    Gtk::ComboBoxText primary_cb;
    Gtk::ComboBoxText secondary_cb;

  protected:
    void fill_store (void);
    void clear_filter (void);
    bool have_prerequisites_for_skill (ApiSkill const* skill);

  public:
    GtkSkillBrowser (void);
};

/* ---------------------------------------------------------------- */

class GtkCertBrowser : public ItemBrowserBase, public Gtk::Box
{
  protected:
    enum CertPrerequisite
    {
      CERT_PRE_HAVE_NONE,
      CERT_PRE_HAVE_SOME,
      CERT_PRE_HAVE_ALL
    };

  private:
    Gtk::Entry filter_entry;
    Gtk::ComboBoxText filter_cb;

  protected:
    void fill_store (void);
    void clear_filter (void);
    CertPrerequisite check_prerequisites_for_cert (ApiCert const* cert);

  public:
    GtkCertBrowser (void);
};

/* ---------------------------------------------------------------- */

inline SignalApiElementSelected&
ItemBrowserBase::signal_element_selected (void)
{
  return this->sig_element_selected;
}

inline SignalApiElementActivated&
ItemBrowserBase::signal_element_activated (void)
{
  return this->sig_element_activated;
}

inline SignalPlanningRequested&
ItemBrowserBase::signal_planning_requested (void)
{
  return this->sig_planning_requested;
}

inline void
ItemBrowserBase::on_planning_requested (ApiElement const* elem, int level)
{
  this->sig_planning_requested.emit(elem, level);
}

inline void
ItemBrowserBase::set_character (ApiCharSheetPtr charsheet)
{
  this->charsheet = charsheet;
  this->fill_store();
}

#endif /* GTK_ITEM_BROWSER_HEADER */
