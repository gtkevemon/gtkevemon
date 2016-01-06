// This file is part of GtkEveMon.
//
// GtkEveMon is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// You should have received a copy of the GNU General Public License
// along with GtkEveMon. If not, see <http://www.gnu.org/licenses/>.

#include <iostream>

#include <gtkmm.h>

#include "util/exception.h"
#include "api/apicharlist.h"
#include "bits/config.h"
#include "bits/characterlist.h"
#include "gtkdefines.h"
#include "gtkhelpers.h"
#include "guiuserdata.h"

GuiUserData::GuiUserData (void)
  : apply_button("Apply")
{
  /* Setup the EVE API fetcher. */
  this->charlist_fetcher.set_doctype(API_DOCTYPE_CHARLIST);
  this->charlist_fetcher.signal_done().connect(sigc::mem_fun
      (*this, &GuiUserData::on_charlist_available));

  /* Create the store for the tree view. */
  this->char_store = Gtk::ListStore::create(this->char_cols);
  this->char_list.set_model(this->char_store);
  this->char_list.append_column_editable(" ", this->char_cols.selected);
  this->char_list.append_column("Name", this->char_cols.name);
  this->char_list.append_column("ID", this->char_cols.charid);
  this->char_list.append_column("Corporation", this->char_cols.corp);

  this->combo_store = Gtk::ListStore::create(this->combo_cols);
  this->history_box.set_model(this->combo_store);
  this->history_box.pack_start(this->combo_cols.userid, true);
  this->history_box.pack_start(this->combo_cols.chars, false);

  /* Build GUI. */
  Gtk::Label* history_label = MK_LABEL("API Keys:");
  Gtk::Box* history_hbox = MK_HBOX(5);
  history_hbox->pack_start(*history_label, false, false, 0);
  history_hbox->pack_start(this->history_box, true, true, 0);

  Gtk::Box* info1_hbox = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 10));
  Gtk::Image* info1_image = MK_IMG0;
  info1_image->set_from_icon_name("dialog-information",
      Gtk::ICON_SIZE_DIALOG);
  info1_hbox->pack_start(*info1_image, false, false, 0);

  Gtk::Label* info1_label = MK_LABEL(
      "Enter your key ID and your verification code such that GtkEveMon "
      "can request \"SkillQueue\" and \"CharacterSheet\". You can get "
      "the information at:\n"
      "<a href='https://community.eveonline.com/support/api-key/'>"
      "https://community.eveonline.com/support/api-key/</a>");
  info1_label->set_line_wrap(true);
  info1_label->set_halign(Gtk::ALIGN_START);
  info1_label->set_use_markup(true);
  Gtk::Box* info1_vbox = MK_VBOX(0);
  info1_vbox->pack_start(*info1_label, false, false, 0);
  info1_hbox->pack_start(*info1_vbox, true, true, 0);

  this->api_v1_cb.set_label("APIv1");
  Gtk::Table* data_table = Gtk::manage(new Gtk::Table(2, 3));
  data_table->set_row_spacings(5);
  data_table->set_col_spacings(5);
  Gtk::Label* userid_label = MK_LABEL("Key ID:");
  Gtk::Label* apikey_label = MK_LABEL("V-Code:");
  userid_label->set_halign(Gtk::ALIGN_START);
  apikey_label->set_halign(Gtk::ALIGN_START);
  data_table->attach(*userid_label, 0, 1, 0, 1, Gtk::FILL, Gtk::FILL);
  data_table->attach(*apikey_label, 0, 1, 1, 2, Gtk::FILL, Gtk::FILL);
  data_table->attach(this->userid_entry, 1, 2, 0, 1,
      Gtk::FILL|Gtk::EXPAND, Gtk::FILL);
  data_table->attach(this->apikey_entry, 1, 3, 1, 2,
      Gtk::FILL|Gtk::EXPAND, Gtk::FILL);
  data_table->attach(this->api_v1_cb, 2, 3, 0, 1,
      Gtk::SHRINK, Gtk::SHRINK);

  Gtk::Box* apply_separator = MK_HBOX(5);
  apply_separator->pack_start(*MK_HSEP, true, true, 0);
  apply_separator->pack_start(this->apply_button, false, false, 0);

  Gtk::Box* info2_hbox = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 10));
  Gtk::Image* info2_image = MK_IMG0;
  info2_image->set_from_icon_name("dialog-information",
      Gtk::ICON_SIZE_DIALOG);
  info2_hbox->pack_start(*info2_image, false, false, 0);
  Gtk::Label* info2_label = MK_LABEL(
      "After entering your user information click the button above "
      "to load the character list. Select the characters you want GtkEveMon "
      "to display.");
  info2_label->set_halign(Gtk::ALIGN_START);
  info2_label->set_line_wrap(true);
  info2_hbox->pack_start(*info2_label, true, true, 0);

  Gtk::ScrolledWindow* scwin = Gtk::manage(new Gtk::ScrolledWindow);
  scwin->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_ALWAYS);
  scwin->set_shadow_type(Gtk::SHADOW_ETCHED_IN);
  scwin->add(this->char_list);

  Gtk::Box* settings_vbox = MK_VBOX(5);
  settings_vbox->set_border_width(5);
  settings_vbox->pack_start(*history_hbox, false, false, 0);
  settings_vbox->pack_start(*MK_HSEP, false, false, 0);
  settings_vbox->pack_start(*info1_hbox, false, false, 0);
  settings_vbox->pack_start(*data_table, false, false, 0);
  settings_vbox->pack_start(*apply_separator, false, false, 0);
  settings_vbox->pack_start(*info2_hbox, false, false, 0);
  settings_vbox->pack_start(*scwin, true, true, 0);

  Gtk::Frame* main_frame = MK_FRAME0;
  main_frame->add(*settings_vbox);
  main_frame->set_shadow_type(Gtk::SHADOW_OUT);

  Gtk::Button* but_close = MK_BUT("Close");
  Gtk::Button* but_add = MK_BUT("Add");

  Gtk::Box* button_bar = MK_HBOX(5);
  button_bar->pack_start(*but_close, false, false, 0);
  button_bar->pack_start(*MK_HSEP, true, true, 0);
  button_bar->pack_start(*but_add, false, false, 0);

  Gtk::Box* main_vbox = MK_VBOX(5);
  main_vbox->set_border_width(5);
  main_vbox->pack_start(*main_frame, true, true, 0);
  main_vbox->pack_end(*button_bar, false, false, 0);

  this->set_title("User Data - GtkEveMon");
  this->set_default_size(500, -1);
  this->add(*main_vbox);
  this->show_all();
  info1_label->select_region(0, 0);
  this->userid_entry.grab_focus();

  this->init_from_config();

  this->apply_button.signal_clicked().connect(sigc::mem_fun
      (*this, &GuiUserData::on_apply_clicked));
  but_close->signal_clicked().connect(sigc::mem_fun(*this, &WinBase::close));
  but_add->signal_clicked().connect(sigc::mem_fun
      (*this, &GuiUserData::on_add_clicked));
  this->history_box.signal_changed().connect(sigc::mem_fun
      (*this, &GuiUserData::on_history_selected));
}

/* ---------------------------------------------------------------- */

void
GuiUserData::init_from_config (void)
{
  /* Populate history. */
  this->combo_store->clear();
  ConfSectionPtr accounts = Config::conf.get_section("accounts");
  conf_sections_t::iterator iter = accounts->sections_begin();
  unsigned int acc_amount = 0;
  while (iter != accounts->sections_end())
  {
    Gtk::TreeModel::Row row = *this->combo_store->append();
    row[this->combo_cols.userid] = iter->first;
    row[this->combo_cols.chars] = **iter->second->get_value("chars");
    iter++;
    acc_amount += 1;
  }

  if (acc_amount == 0)
    this->history_box.set_sensitive(false);
}

/* ---------------------------------------------------------------- */

void
GuiUserData::on_charlist_available (EveApiData data)
{
  this->apply_button.set_sensitive(true);

  if (data.data.get() == 0)
  {
    this->print_error(data.exception);
    return;
  }

  ApiCharacterListPtr clist = ApiCharacterList::create();
  try
  {
    clist->set_api_data(data);
    if (clist->is_locally_cached())
      this->print_error(data.exception, true);
  }
  catch (Exception& e)
  {
    this->print_error(e);
    return;
  }

  /* Successfully requested character list. */
  ApiCharList const& chars = clist->chars;
  this->char_store->clear();
  std::string char_string;

  if (chars.empty())
  {
    this->print_error("No characters found for that account!");
    return;
  }

  //std::cout << "Character list has " << chars.size()
  //    << " entries:" << std::endl;
  for (unsigned int i = 0; i < chars.size(); ++i)
  {
    //std::cout << i << ". Character: " << chars[i].name << " ["
    //    << chars[i].char_id << "]" << ", Corp: "
    //    << chars[i].corp << std::endl;

    Gtk::TreeModel::Row row = *this->char_store->append();
    row[this->char_cols.selected] = false;
    row[this->char_cols.name] = chars[i].name;
    row[this->char_cols.charid] = chars[i].char_id;
    row[this->char_cols.corp] = chars[i].corp;

    if (i != 0)
      char_string += ", ";
    char_string += chars[i].name;
  }

  /* Insert the data to the history and the combo box. */
  ConfSectionPtr sect = Config::conf.get_or_create_section
      ("accounts." + this->userid_entry.get_text());
  sect->add("apikey", ConfValue::create(this->apikey_entry.get_text()));
  sect->add("chars", ConfValue::create(char_string));
  sect->add("apiver", ConfValue::create(this->api_v1_cb.get_active()?1:2));
  Config::save_to_file();
}

/* ---------------------------------------------------------------- */

void
GuiUserData::on_apply_clicked (void)
{
  EveApiAuth auth;
  auth.user_id = this->userid_entry.get_text();
  auth.api_key = this->apikey_entry.get_text();
  auth.is_apiv1 = this->api_v1_cb.get_active();

  this->charlist_fetcher.set_auth(auth);
  this->charlist_fetcher.async_request();
  this->apply_button.set_sensitive(false);
}

/* ---------------------------------------------------------------- */

void
GuiUserData::print_error (std::string const& error, bool cached)
{
  Gtk::MessageType message_type;
  Glib::ustring message;
  if (cached)
  {
    message = "Using cached version of the character list!";
    message_type = Gtk::MESSAGE_WARNING;
  }
  else
  {
    message = "Error retrieving character list!";
    message_type = Gtk::MESSAGE_ERROR;
  }

  Gtk::MessageDialog md(*this, message, false, message_type, Gtk::BUTTONS_OK);
  md.set_secondary_text("There was an error while requesting the character "
      "list from the EVE API. The EVE API is either offline, or the "
      "requested document is not understood by GtkEveMon. "
      "The error message is:\n\n" + GtkHelpers::locale_to_utf8(error));
  md.set_title("Error - GtkEveMon");
  md.run();
}

/* ---------------------------------------------------------------- */

void
GuiUserData::on_add_clicked (void)
{
  std::string user_id = this->userid_entry.get_text();
  std::string api_key = this->apikey_entry.get_text();
  bool is_apiv1 = this->api_v1_cb.get_active();

  EveApiAuth auth(user_id, api_key);

  Gtk::TreeModel::Children childs = this->char_store->children();
  for (Gtk::TreeIter iter = childs.begin(); iter != childs.end(); iter++)
  {
    bool selected = (*iter)[this->char_cols.selected];
    if (!selected)
      continue;

    Glib::ustring charid = (*iter)[this->char_cols.charid];
    auth.char_id = charid;
    auth.is_apiv1 = is_apiv1;
    CharacterList::request()->add_character(auth);
  }

  this->close();
}

/* ---------------------------------------------------------------- */

void
GuiUserData::on_history_selected (void)
{
  Gtk::TreeModel::iterator iter = this->history_box.get_active();
  if (iter)
  {
    Gtk::TreeModel::Row row = *iter;
    Glib::ustring userid = row[this->combo_cols.userid];
    ConfSectionPtr sect = Config::conf.get_section
        ("accounts." + userid);
    std::string apikey = **sect->get_value("apikey");

    bool is_apiv1 = true;
    try { is_apiv1 = sect->get_value("apiver")->get_int() == 1; }
    catch (...) {}

    this->userid_entry.set_text(userid);
    this->apikey_entry.set_text(apikey);
    this->api_v1_cb.set_active(is_apiv1);
    this->char_store->clear();
  }
}
