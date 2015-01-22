#include <iostream>
#include <sstream>
#include <gtkmm/button.h>
#include <gtkmm/main.h>
#include <gtkmm/frame.h>
#include <gtkmm/box.h>
#include <gtkmm/separator.h>
#include <gtkmm/stock.h>
#include <gtkmm/messagedialog.h>

#include "util/helpers.h"
#include "api/evetime.h"
#include "api/eveapi.h"
#include "api/implant.h"
#include "bits/config.h"
#include "bits/server.h"
#include "bits/serverlist.h"
#include "bits/argumentsettings.h"
#include "imagestore.h"
#include "gtkdefines.h"
#include "gtkserver.h"
#include "gtkcharpage.h"
#include "guiupdater.h"
#include "guiuserdata.h"
#include "guiconfiguration.h"
#include "guiaboutdialog.h"
#include "guievelauncher.h"
#include "guiskillplanner.h"
#include "guixmlsource.h"
#include "guicharexport.h"
#include "maingui.h"

MainGui::MainGui (void)
  : info_display(INFO_STYLE_FRAMED), iconified(false)
{
  this->conf_windowtitle = Config::conf.get_value("settings.verbose_wintitle");
  this->conf_detailed_tooltip = Config::conf.get_value
      ("settings.detailed_tray_tooltip");
  this->updater = new Updater();
  this->notebook.set_scrollable(true);

  /* Create the actions, menus and toolbars. */
  this->actions = Gtk::ActionGroup::create();
  this->uiman = Gtk::UIManager::create();

  this->actions->add(Gtk::Action::create("MenuEveMon",
      Gtk::Stock::OK, "_EveMon"));
  this->actions->add(Gtk::Action::create("SetupProfile",
      Gtk::Stock::ADD, "_Add characters..."),
      sigc::mem_fun(*this, &MainGui::setup_profile));
  this->actions->add(Gtk::Action::create("Configuration",
      Gtk::Stock::PREFERENCES, "_Configuration..."),
      sigc::mem_fun(*this, &MainGui::configuration));
  this->actions->add(Gtk::Action::create("CheckUpdates",
      Gtk::Stock::NETWORK, "Check for _updates..."),
      sigc::mem_fun(*this, &MainGui::version_checker));
  this->actions->add(Gtk::Action::create("QuitEveMon", Gtk::Stock::QUIT),
      sigc::mem_fun(*this, &MainGui::close));
  this->actions->add(Gtk::Action::create("LaunchEVE",
      Gtk::Stock::EXECUTE, "_Launch EVE-Online"),
      sigc::mem_fun(*this, &MainGui::launch_eve));

  this->actions->add(Gtk::Action::create("MenuCharacter",
      Gtk::Stock::JUSTIFY_FILL, "_Character"));
  this->actions->add(Gtk::Action::create("MenuCharPlanning",
      Gtk::Stock::OK, "_Training planner..."),
      sigc::mem_fun(*this, &MainGui::create_skillplan));
  this->actions->add(Gtk::Action::create("MenuCharXMLSource",
      Gtk::Stock::OK, "View _XML source..."),
      sigc::mem_fun(*this, &MainGui::view_xml_source));
  this->actions->add(Gtk::Action::create("MenuCharInfoExport",
      Gtk::Stock::PRINT, "_Export character..."),
      sigc::mem_fun(*this, &MainGui::export_char_info));

  this->actions->add(Gtk::Action::create("MenuHelp",
      Gtk::Stock::HELP, "_Help"));
  this->actions->add(Gtk::Action::create("AboutDialog",
      Gtk::Stock::ABOUT, "_About..."),
      sigc::mem_fun(*this, &MainGui::about_dialog));

  this->uiman->insert_action_group(this->actions);
  this->add_accel_group(this->uiman->get_accel_group());

  char const* ui_string =
      "<ui>"
      "  <menubar name='MenuBar'>"
      "    <menu name='MenuEveMon' action='MenuEveMon'>"
      "      <menuitem action='SetupProfile'/>"
      "      <menuitem action='Configuration'/>"
      "      <menuitem action='CheckUpdates' />"
      "      <separator/>"
      "      <menuitem action='LaunchEVE'/>"
      "      <separator/>"
      "      <menuitem action='QuitEveMon'/>"
      "    </menu>"
      "    <menu name='MenuCharacter' action='MenuCharacter'>"
      "      <menuitem action='MenuCharPlanning'/>"
      "      <menuitem action='MenuCharXMLSource'/>"
      "      <menuitem action='MenuCharInfoExport'/>"
      "    </menu>"
      "    <menu name='MenuHelp' action='MenuHelp'>"
      "      <menuitem action='AboutDialog' />"
      "    </menu>"
      "  </menubar>"
      "</ui>";

  #ifdef GLIBMM_EXCEPTIONS_ENABLED
  this->uiman->add_ui_from_string(ui_string);
  #else
  std::auto_ptr<Glib::Error> error;
  this->uiman->add_ui_from_string(ui_string, error);
  if (error.get())
    throw error;
  #endif

  Gtk::Widget* menu_bar = this->uiman->get_widget("/MenuBar");

  /* Set icon for the EveMon menu. */
  Gtk::ImageMenuItem* tmp_item = (Gtk::ImageMenuItem*)
      this->uiman->get_widget("/MenuBar/MenuEveMon");
  tmp_item->set_image(*Gtk::manage(new Gtk::Image(ImageStore::menuicons[0])));

  tmp_item = (Gtk::ImageMenuItem*)this->uiman->get_widget
      ("/MenuBar/MenuCharacter");
  tmp_item->set_image(*Gtk::manage(new Gtk::Image(ImageStore::menuicons[1])));

  tmp_item = (Gtk::ImageMenuItem*)this->uiman->get_widget("/MenuBar/MenuHelp");
  tmp_item->set_image(*Gtk::manage(new Gtk::Image(ImageStore::menuicons[2])));

  tmp_item = (Gtk::ImageMenuItem*)this->uiman->get_widget
      ("/MenuBar/MenuCharacter/MenuCharPlanning");
  tmp_item->set_image(*Gtk::manage(new Gtk::Image
      (ImageStore::skill->scale_simple(16, 16, Gdk::INTERP_BILINEAR))));

  /* Set the Stock item separately to avoid getting a shortcut key. */
  tmp_item = (Gtk::ImageMenuItem*)this->uiman->get_widget
      ("/MenuBar/MenuCharacter/MenuCharXMLSource");
  tmp_item->set_image(*Gtk::manage(new Gtk::Image
      (Gtk::Stock::FIND, Gtk::ICON_SIZE_MENU)));

  /* Right-justify the help menu. */
  Gtk::MenuItem* help_menu = (Gtk::MenuItem*)this->uiman->get_widget
      ("/MenuBar/MenuHelp");
  help_menu->set_right_justified(true);

  /* Create the GUI part of server list. */
  Gtk::HBox* server_box = Gtk::manage(new Gtk::HBox(false, 5));
  server_box->pack_start(*MK_LABEL0, true, true, 0);

  for (unsigned int i = 0; i < ServerList::list.size(); ++i)
  {
    GtkServer* server = Gtk::manage(new GtkServer(ServerList::list[i]));
    server_box->pack_start(*server, false, false, 0);
    if (i != ServerList::list.size() - 1)
      server_box->pack_start(*MK_VSEP, false, false, 0);
    this->gtkserver.push_back(server);
  }
  server_box->pack_start(*MK_LABEL0, true, true, 0);

  /* The box with the local and EVE time. */
  Gtk::HBox* time_hbox = MK_HBOX;
  time_hbox->pack_start(this->evetime_label, false, false, 0);
  time_hbox->pack_end(this->localtime_label, false, false, 0);

  /* Build the server monitor with the time box. Ommit the server
   * box if there are no servers to monitor. */
  Gtk::VBox* server_info_box = Gtk::manage(new Gtk::VBox(false, 2));
  server_info_box->set_border_width(5);
  if (ServerList::list.size() > 0)
  {
    server_info_box->pack_start(*server_box, false, false, 0);
    server_info_box->pack_start(*MK_HSEP, false, false, 0);
  }
  server_info_box->pack_start(*time_hbox, false, false, 0);

  Gtk::Frame* server_frame = MK_FRAME0;
  server_frame->set_shadow_type(Gtk::SHADOW_OUT);
  server_frame->add(*server_info_box);

  /* Pack content area together. */
  Gtk::VBox* content_vbox = MK_VBOX;
  content_vbox->set_border_width(5);
  content_vbox->pack_start(this->notebook, true, true, 0);
  content_vbox->pack_end(*server_frame, false, false, 0);

  /* Pack window contents together. */
  Gtk::VBox* main_vbox = MK_VBOX0;
  main_vbox->pack_start(*menu_bar, false, false, 0);
  main_vbox->pack_start(this->info_display, false, false, 0);
  main_vbox->pack_start(*content_vbox, true, true, 0);

  /* Setup window stuff. */
  this->set_icon(ImageStore::applogo);
  this->set_title("GtkEveMon");
  this->set_default_size(550, 640);
  this->add(*main_vbox);
  this->show_all();
  if (ArgumentSettings::start_minimized) this->iconify();
  this->info_display.hide();

  /* Connect signals. */
  this->signal_window_state_event().connect(sigc::mem_fun
      (*this, &MainGui::on_window_state_event));
  this->notebook.signal_page_added().connect(sigc::mem_fun
      (*this, &MainGui::on_pages_changed));
  this->notebook.signal_page_removed().connect(sigc::mem_fun
      (*this, &MainGui::on_pages_changed));
  this->notebook.signal_switch_page().connect(sigc::mem_fun
      (*this, &MainGui::on_pages_switched));
  this->updater->signal_files_changed().connect(sigc::mem_fun
      (*this, &MainGui::on_data_files_changed));
  this->updater->signal_files_unchanged().connect(sigc::mem_fun
      (*this, &MainGui::on_data_files_unchanged));

  /* Connect signals of the character list. */
  CharacterListPtr charlist = CharacterList::request();
  charlist->signal_char_added().connect
      (sigc::mem_fun(*this, &MainGui::add_character));
  charlist->signal_char_removed().connect
      (sigc::mem_fun(*this, &MainGui::remove_character));

  /* Setup timers for refresh and GUI update for the servers. */
  Glib::signal_timeout().connect(sigc::mem_fun
      (*this, &MainGui::refresh_servers), MAINGUI_SERVER_REFRESH);
  Glib::signal_timeout().connect(sigc::mem_fun
      (*this, &MainGui::update_time), MAINGUI_TIME_UPDATE);
  Glib::signal_timeout().connect(sigc::mem_fun
      (*this, &MainGui::update_tooltip), MAINGUI_TOOLTIP_UPDATE);
  Glib::signal_timeout().connect(sigc::mem_fun(*this,
      &MainGui::update_windowtitle), MAINGUI_WINDOWTITLE_UPDATE);

  this->update_time();
  this->init_from_charlist();
  this->updater->background_check_async();
}

/* ---------------------------------------------------------------- */

bool
MainGui::refresh_servers (void)
{
  ServerList::refresh();
  return true;
}

/* ---------------------------------------------------------------- */

bool
MainGui::update_tooltip (void)
{
  if (!this->tray)
    return true;

  if (!this->notebook.get_show_tabs())
    return true;

  bool detailed = this->conf_detailed_tooltip->get_bool();
  std::string tooltip;

  CharacterListPtr clist = CharacterList::request();
  for (std::size_t i = 0; i < clist->chars.size(); ++i)
  {
    std::string char_tt = clist->chars[i]->get_summary_text(detailed);
    if (!char_tt.empty())
    {
      if (i != 0)
        tooltip += "\n";
      tooltip += char_tt;
    }
  }

  this->tray->set_tooltip(tooltip);
  return true;
}

/* ---------------------------------------------------------------- */

void
MainGui::init_from_charlist (void)
{
  CharacterListPtr clist = CharacterList::request();
  for (std::size_t i = 0; i < clist->chars.size(); ++i)
    this->add_character(clist->chars[i]);

  this->check_if_no_pages();
}

/* ---------------------------------------------------------------- */

void
MainGui::add_character (CharacterPtr character)
{
  /* If tabs are not shown, the welcome page is visible. */
  if (this->notebook.get_show_tabs() == false)
  {
    this->notebook.pages().clear();
    this->notebook.set_show_tabs(true);
  }

  /* Create the new character page for the notebook. */
  GtkCharPage* page = Gtk::manage(new GtkCharPage(character));
  page->set_parent_window(this);
  this->notebook.append_page(*page, character->get_char_name(), false);
  this->notebook.set_current_page(-1);

  character->signal_name_available().connect(sigc::mem_fun
      (*this, &MainGui::update_char_name));

  /* Update tray icon tooltips. */
  this->update_tooltip();
}

/* ---------------------------------------------------------------- */

void
MainGui::remove_character (std::string char_id)
{
  Glib::ListHandle<Gtk::Widget*> childs = this->notebook.get_children();
  for (Glib::ListHandle<Gtk::Widget*>::iterator iter = childs.begin();
      iter != childs.end(); iter++)
  {
    CharacterPtr character = ((GtkCharPage*)*iter)->get_character();
    if (character->get_char_id() == char_id)
    {
      this->notebook.remove_page(**iter);
      break;
    }
  }

  this->check_if_no_pages();
  this->update_tooltip();
}

/* ---------------------------------------------------------------- */

void
MainGui::check_if_no_pages (void)
{
  if (this->notebook.pages().empty())
  {
    Gtk::HBox* info_hbox = Gtk::manage(new Gtk::HBox(false, 15));
    Gtk::Image* info_image = Gtk::manage(new Gtk::Image
        (Gtk::Stock::DIALOG_INFO, Gtk::ICON_SIZE_DIALOG));
    info_image->set_alignment(Gtk::ALIGN_RIGHT);
    Gtk::Label* info_label = MK_LABEL
        ("GtkEveMon needs to connect to the EVE API in order to "
        "retrieve information about your character. "
        "For this operation, GtkEveMon needs your user ID and API key. "
        "You also need to select some characters to be monitored. "
        "Go ahead and add some characters.");
    info_label->set_line_wrap(true);
    info_label->set_alignment(Gtk::ALIGN_LEFT);
    info_hbox->pack_start(*info_image, true, true, 0);
    info_hbox->pack_start(*info_label, true, true, 0);

    Gtk::HBox* button_hbox = MK_HBOX;
    Gtk::Button* add_characters_but = Gtk::manage
        (new Gtk::Button("Add characters"));
    add_characters_but->set_image(*Gtk::manage
        (new Gtk::Image(Gtk::Stock::ADD, Gtk::ICON_SIZE_BUTTON)));

    button_hbox->pack_start(*MK_HSEP, true, true, 0);
    button_hbox->pack_start(*add_characters_but, false, false, 0);
    button_hbox->pack_end(*MK_HSEP, true, true, 0);

    Gtk::VBox* upper_vbox = MK_VBOX0;
    upper_vbox->pack_end(*info_hbox, false, false, 0);
    Gtk::VBox* bottom_vbox = MK_VBOX0;
    bottom_vbox->pack_start(*button_hbox, false, false, 0);

    Gtk::VBox* main_vbox = Gtk::manage(new Gtk::VBox(false, 15));
    main_vbox->set_border_width(5);
    main_vbox->pack_start(*upper_vbox, true, true, 0);
    main_vbox->pack_start(*bottom_vbox, true, true, 0);
    main_vbox->show_all();

    add_characters_but->signal_clicked().connect
        (sigc::mem_fun(*this, &MainGui::setup_profile));

    this->notebook.set_show_tabs(false);
    this->notebook.append_page(*main_vbox, "Getting started...");
  }
}

/* ---------------------------------------------------------------- */

bool
MainGui::update_time (void)
{
  std::string evetime;
  if (EveTime::is_initialized())
    this->evetime_label.set_text("EVE time: " + EveTime::get_eve_time_string());
  else
    this->evetime_label.set_text("EVE time: Not yet known!");

  this->localtime_label.set_text("Local time: "
      + EveTime::get_local_time_string());

  return true;
}

/* ---------------------------------------------------------------- */

bool
MainGui::update_windowtitle (void)
{
  int current = this->notebook.get_current_page();
  if (current < 0
      || !this->notebook.get_show_tabs()
      || !this->conf_windowtitle->get_bool())
  {
    this->set_title("GtkEveMon");
    return true;
  }

  GtkCharPage* page = (GtkCharPage*)this->notebook.pages()[current].get_child();
  CharacterPtr character = page->get_character();
  Glib::ustring title;

  title.append(character->get_char_name());
  title.append(": ");
  title.append(character->get_remaining_text(true));
  title.append(" - GtkEveMon");

  this->set_title(title);

  return true;
}

/* ---------------------------------------------------------------- */
/* FIXME: What is the return value? */

bool
MainGui::on_window_state_event (GdkEventWindowState* event)
{
  ConfValuePtr value = Config::conf.get_value("settings.tray_usage");

  /* Manage the tray icon. */
  if (**value == "minimize")
  {
    if (event->new_window_state & GDK_WINDOW_STATE_ICONIFIED)
      this->create_tray_icon();
    else
      this->destroy_tray_icon();
  }
  else if (**value == "always")
  {
    if (!this->tray)
      this->create_tray_icon();
  }
  else /* if (**value == "never") */
  {
    if (this->tray)
      this->destroy_tray_icon();
  }

  /* Manage window state. */
  if (event->new_window_state & GDK_WINDOW_STATE_ICONIFIED)
  {
    this->iconified = true;

    if (**value != "never")
    {
      this->set_skip_taskbar_hint(true);
      //this->hide();
    }
  }
  else
  {
    this->iconified = false;
    this->set_skip_taskbar_hint(false);
    //this->show();
  }

  while (Gtk::Main::events_pending())
    Gtk::Main::iteration();

  return true;
}

/* ---------------------------------------------------------------- */

void
MainGui::on_tray_icon_clicked (void)
{
  //Gdk::WindowState ws = this->get_screen()->get_active_window()->get_state();
  //std::cout << "WS: " << ws << std::endl;
  //if ((ws & Gdk::WINDOW_STATE_ICONIFIED) == Gdk::WINDOW_STATE_ICONIFIED)

  //std::cout << "Tray clicked. Taskbar hint is: " << this->get_skip_taskbar_hint() << std::endl;

  if (this->iconified)
  {
    this->set_skip_taskbar_hint(false);
    this->deiconify();
    this->iconified = false;
  }
  else
  {
    this->iconify();
    this->iconified = true;
  }
}

/* ---------------------------------------------------------------- */

bool
MainGui::on_delete_event (GdkEventAny* /*event*/)
{
  if (Config::conf.get_value("settings.minimize_on_close")->get_bool())
    this->iconify();
  else
    this->close();

  return true;
}

/* ---------------------------------------------------------------- */

void
MainGui::on_pages_changed (Gtk::Widget* /*widget*/, guint /*pnum*/)
{
  Gtk::MenuItem* char_menu = (Gtk::MenuItem*)this->uiman->get_widget
      ("/MenuBar/MenuCharacter");

  if (char_menu == 0)
    return;

  char_menu->set_sensitive(this->notebook.get_show_tabs());
}

/* ---------------------------------------------------------------- */

void
MainGui::on_pages_switched (GtkNotebookPage* /*page*/, guint /*pnum*/)
{
  this->update_windowtitle();
}

/* ---------------------------------------------------------------- */

void
MainGui::on_data_files_changed (void)
{
  this->info_display.append(INFO_NOTIFICATION,
      "The data files have been updated. Please restart GtkEveMon!",
      "The data files updater just updated SkillTree.xml and "
      "CertificateTree.xml. Please restart GtkEveMon to use the updated "
      "files. You can disable the automatic updater in the options.");
  delete this->updater;
}

/* ---------------------------------------------------------------- */

void
MainGui::on_data_files_unchanged (void)
{
  delete this->updater;
}

/* ---------------------------------------------------------------- */

void
MainGui::update_char_name (std::string char_id)
{
  Glib::ListHandle<Gtk::Widget*> childs = this->notebook.get_children();
  for (Glib::ListHandle<Gtk::Widget*>::iterator iter = childs.begin();
      iter != childs.end(); iter++)
  {
    CharacterPtr tmp_char = ((GtkCharPage*)*iter)->get_character();
    if (tmp_char->get_char_id() == char_id)
    {
      GtkCharPage* page = (GtkCharPage*)*iter;
      this->notebook.set_tab_label_text(*page, tmp_char->get_char_name());
    }
  }

  this->update_tooltip();
}

/* ---------------------------------------------------------------- */

void
MainGui::tray_popup_menu (guint button, guint32 activate_time)
{
  Gtk::MenuItem* mi = (Gtk::MenuItem*)this->uiman->get_widget
      ("/MenuBar/MenuEveMon");
  Gtk::Menu* m = mi->get_submenu();
  m->popup(button, activate_time);
}

/* ---------------------------------------------------------------- */

void
MainGui::update_tray_settings (void)
{
  ConfValuePtr value = Config::conf.get_value("settings.tray_usage");

  if (**value == "never")
  {
    this->destroy_tray_icon();
  }
  else if (**value == "always")
  {
    this->create_tray_icon();
  }
  else if (**value == "minimize")
  {
    this->destroy_tray_icon();
  }
}

/* ---------------------------------------------------------------- */

void
MainGui::create_tray_icon (void)
{
  if (this->tray)
    return;

  /* Create the tray icon. */
  this->tray = Gtk::StatusIcon::create(ImageStore::applogo);
  this->tray->signal_activate().connect(sigc::mem_fun
      (*this, &MainGui::on_tray_icon_clicked));
  this->tray->signal_popup_menu().connect(sigc::mem_fun
      (*this, &MainGui::tray_popup_menu));
  this->update_tooltip();
}

/* ---------------------------------------------------------------- */

void
MainGui::destroy_tray_icon (void)
{
  /* Destroy the tray icon. */
  //this->tray.reset(); // Compile error for glibmm < 2.16
  this->tray.clear(); // Deprecated
}

/* ---------------------------------------------------------------- */

void
MainGui::setup_profile (void)
{
  GuiUserData* dialog = new GuiUserData();
  dialog->set_transient_for(*this);
}

/* ---------------------------------------------------------------- */

void
MainGui::configuration (void)
{
  GuiConfiguration* dialog = new GuiConfiguration();
  dialog->set_transient_for(*this);
  dialog->signal_tray_settings_changed().connect(sigc::mem_fun
      (*this, &MainGui::update_tray_settings));
}

/* ---------------------------------------------------------------- */

void
MainGui::about_dialog (void)
{
  Gtk::Window* dialog = new GuiAboutDialog();
  dialog->set_transient_for(*this);
}

/* ---------------------------------------------------------------- */

void
MainGui::version_checker (void)
{
  GuiUpdater* updater = new GuiUpdater(false);
  updater->set_transient_for(*this);
}

/* ---------------------------------------------------------------- */

void
MainGui::launch_eve (void)
{
  try
  {
    Gtk::Window* win = GuiEveLauncher::launch_eve();
    if (win != 0)
      win->set_transient_for(*this);
  }
  catch (Exception& e)
  {
    this->info_display.append(INFO_WARNING, e);
  }
}

/* ---------------------------------------------------------------- */

void
MainGui::create_skillplan (void)
{
  int current = this->notebook.get_current_page();
  if (current < 0)
    return;
  CharacterPtr character = CharacterList::request()->chars[current];

  if (!character->valid_character_sheet() || !character->valid_training_sheet())
  {
    this->info_display.append(INFO_WARNING, "Cannot open the skill "
        "planner without valid character sheets!");
    return;
  }

  GuiSkillPlanner* planner = new GuiSkillPlanner();
  planner->set_character(character);
}

/* ---------------------------------------------------------------- */

void
MainGui::view_xml_source (void)
{
  int current = this->notebook.get_current_page();
  if (current < 0)
    return;
  CharacterPtr character = CharacterList::request()->chars[current];

  ApiCharSheetPtr cs = character->cs;
  ApiSkillQueuePtr sq = character->sq;
  ImplantsPtr imp = Implants::request();
  
  if (!cs->valid && !sq->valid && !imp->valid)
  {
    this->info_display.append(INFO_WARNING, "Cannot open the source "
        "viewer without a valid sheet!");
    return;
  }

  GuiXmlSource* window = new GuiXmlSource();

  if (cs->valid)
    window->append(cs->get_http_data(), "CharacterSheet.xml");
  if (sq->valid)
    window->append(sq->get_http_data(), "SkillQueue.xml");
  if (imp->valid)
    window->append(imp->get_data(), "Implants.xml");
}

/* ---------------------------------------------------------------- */

void
MainGui::export_char_info (void)
{
  int current = this->notebook.get_current_page();
  if (current < 0)
    return;
  CharacterPtr character = CharacterList::request()->chars[current];

  if (!character->cs->valid)
  {
    this->info_display.append(INFO_WARNING, "Cannot open the exporter "
        "without valid character sheet!");
    return;
  }

  new GuiCharExport(character->cs);
}

/* ---------------------------------------------------------------- */

void
MainGui::close (void)
{
  Gtk::Main::quit();
}
