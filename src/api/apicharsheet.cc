#include <cmath>
#include <iostream>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include "util/exception.h"
#include "util/helpers.h"
#include "xml.h"
#include "apibase.h"
#include "apiskilltree.h"
#include "apicerttree.h"
#include "apicharsheet.h"

void
ApiCharSheet::set_api_data (EveApiData const& data)
{
  this->valid = false;
  this->ApiBase::set_api_data(data);

  /* Reset values. */
  this->implant = 0.0;
  this->total_sp = 0;
  for (int i = 0; i < 6; ++i)
    this->skills_at[i] = 0;

  /* Parse the data. */
  this->parse_xml();

  /* Force the sheet to have at least 30 minutes cache time. */
  this->enforce_cache_time(API_CHAR_SHEET_MIN_CACHE_TIME);

  /* Find bonus attributes for skills. */
  this->total = this->base + this->implant;

  /* Calculate start SP, destination SP and completed. */
  ApiSkillTreePtr stree = ApiSkillTree::request();
  for (std::size_t i = 0; i < this->skills.size(); ++i)
  {
    ApiCharSheetSkill& cskill = this->skills[i];
    ApiSkill const* skill = stree->get_skill_for_id(cskill.id);
    if (skill == 0)
    {
      std::cout << "Warning: Ignoring unknown skill (ID " << cskill.id
          << "). Update SkillTree.xml." << std::endl;
      this->skills.erase(this->skills.begin() + i);
      i -= 1;
      continue;
    }

    /* Update a few fields of the char sheet skill. */
    cskill.details = skill;
    cskill.points_start = ApiCharSheet::calc_start_sp
        (skills[i].level, skill->rank);
    cskill.points_dest = ApiCharSheet::calc_dest_sp
        (skills[i].level, skill->rank);
    cskill.points_max = ApiCharSheet::calc_dest_sp
        (4, skill->rank);

    cskill.completed = (double)(skills[i].points - skills[i].points_start)
        / (double)(skills[i].points_dest - skills[i].points_start);

    /* Sum up the total amount of SP for the character. */
    this->total_sp += cskill.points;
    this->skills_at[cskill.level] += 1;
  }

  /* Update certificate field "details". */
  ApiCertTreePtr ctree = ApiCertTree::request();
  for (unsigned int i = 0; i < this->certs.size(); ++i)
  {
    ApiCharSheetCert& ccert = this->certs[i];
    ApiCert const* cert = ctree->get_certificate_for_id(ccert.id);
    if (cert == 0)
    {
      std::cout << "Warning: Ignoring unknown certificate (ID " << ccert.id
          << "). Update GtkEveMon or CertificateTree.xml." << std::endl;
      this->certs.erase(this->certs.begin() + i);
      i -= 1;
      continue;
    }

    ccert.details = cert;
  }

  //this->debug_dump();
  this->valid = true;
}

/* ---------------------------------------------------------------- */

void
ApiCharSheet::parse_xml (void)
{
  this->skills.clear();

  std::cout << "Parsing XML: CharacterSheet.xml ..." << std::endl;
  XmlDocumentPtr xml = XmlDocument::create
      (&this->http_data->data[0], this->http_data->data.size());
  xmlNodePtr root = xml->get_root_element();
  this->parse_eveapi_tag(root);
}

/* ---------------------------------------------------------------- */

void
ApiCharSheet::parse_eveapi_tag (xmlNodePtr node)
{
  if (node->type != XML_ELEMENT_NODE
      || xmlStrcmp(node->name, (xmlChar const*)"eveapi"))
    throw Exception("Invalid XML root. Expecting <eveapi> node.");

  for (node = node->children; node != 0; node = node->next)
  {
    /* Let the base class know of some fields. */
    this->check_node(node);

    if (node->type == XML_ELEMENT_NODE
        && !xmlStrcmp(node->name, (xmlChar const*)"result"))
    {
      //std::cout << "Found <result> tag" << std::endl;
      this->parse_result_tag(node->children);
    }
  }
}

/* ---------------------------------------------------------------- */

void
ApiCharSheet::parse_result_tag (xmlNodePtr node)
{
  for (; node != 0; node = node->next)
  {
    if (node->type != XML_ELEMENT_NODE)
      continue;

    this->set_string_if_node_text(node, "characterID", this->char_id);
    this->set_string_if_node_text(node, "name", this->name);
    this->set_string_if_node_text(node, "race", this->race);
    this->set_string_if_node_text(node, "bloodLine", this->bloodline);
    this->set_string_if_node_text(node, "gender", this->gender);
    this->set_string_if_node_text(node, "corporationName", this->corp);
    this->set_string_if_node_text(node, "balance", this->balance);

    this->set_string_if_node_text(node, "cloneName", this->clone_name);
    this->set_uint_if_node_text(node, "cloneSkillPoints", this->clone_sp);
    this->set_uint_if_node_text(node, "freeSkillPoints", this->free_sp);
    this->set_string_if_node_text(node, "lastRespecDate", this->last_respec);
    this->set_string_if_node_text(node, "lastTimedRespec", this->last_timed_respec);
    this->set_uint_if_node_text(node, "freeRespecs", this->free_respecs);
    this->set_string_if_node_text(node, "cloneJumpDate", this->last_clone_jump);

    if (!xmlStrcmp(node->name, (xmlChar const*)"attributes"))
      this->parse_attribute_tag(node->children);

    if (!xmlStrcmp(node->name, (xmlChar const*)"attributeEnhancers"))
      this->parse_attrib_enhancers_tag(node->children);

    if (!xmlStrcmp(node->name, (xmlChar const*)"rowset"))
    {
      std::string name = this->get_property(node, "name");
      if (name == "skills")
        this->parse_skills_tag(node->children);
      else if (name == "certificates")
        this->parse_certificates_tag(node->children);
    }
  }
}

/* ---------------------------------------------------------------- */

void
ApiCharSheet::parse_attribute_tag (xmlNodePtr node)
{
  for (; node != 0; node = node->next)
  {
    if (node->type != XML_ELEMENT_NODE)
      continue;

    this->set_double_if_node_text(node, "intelligence", this->base.intl);
    this->set_double_if_node_text(node, "memory", this->base.mem);
    this->set_double_if_node_text(node, "charisma", this->base.cha);
    this->set_double_if_node_text(node, "perception", this->base.per);
    this->set_double_if_node_text(node, "willpower", this->base.wil);
  }
}

/* ---------------------------------------------------------------- */

void
ApiCharSheet::parse_attrib_enhancers_tag (xmlNodePtr node)
{
  for (; node != 0; node = node->next)
  {
    if (node->type != XML_ELEMENT_NODE)
      continue;

    this->find_implant_bonus(node, "memoryBonus", this->implant.mem);
    this->find_implant_bonus(node, "willpowerBonus", this->implant.wil);
    this->find_implant_bonus(node, "perceptionBonus", this->implant.per);
    this->find_implant_bonus(node, "intelligenceBonus", this->implant.intl);
    this->find_implant_bonus(node, "charismaBonus", this->implant.cha);
  }
}

/* ---------------------------------------------------------------- */

void
ApiCharSheet::parse_skills_tag (xmlNodePtr node)
{
  for (; node != 0; node = node->next)
  {
    if (node->type != XML_ELEMENT_NODE)
      continue;

    if (!xmlStrcmp(node->name, (xmlChar const*)"row"))
    {
      /* Prepare a new skill to append. */
      ApiCharSheetSkill skill;

      /* Fetch these fields. If they are not there, it's an error. */
      try
      {
        skill.id = Helpers::get_int_from_string
            (this->get_property(node, "typeID"));
        skill.points = Helpers::get_int_from_string
            (this->get_property(node, "skillpoints"));
      }
      catch (Exception& e)
      {
        throw Exception(e + " in element \"row\"");
      }

      /* Fetch the level field. If it's not there, ignore the skill.
       * This might happen if the skill in "unpublished" (removed?). */
      try
      {
        skill.level = Helpers::get_int_from_string
            (this->get_property(node, "level"));

        /* Add skill to list. */
        this->skills.push_back(skill);
      }
      catch (Exception& e)
      {
        std::cout << "Warning: Ignoring skill without "
            "\"level\" attribute: " << skill.id << std::endl;
      }
    }
  }
}

/* ---------------------------------------------------------------- */

void
ApiCharSheet::parse_certificates_tag (xmlNodePtr node)
{
  for (; node != 0; node = node->next)
  {
    if (node->type != XML_ELEMENT_NODE)
      continue;

    if (!xmlStrcmp(node->name, (xmlChar const*)"row"))
    {
      ApiCharSheetCert cert;

      try
      {
        cert.id = Helpers::get_int_from_string
            (this->get_property(node, "certificateID"));
        this->certs.push_back(cert);
      }
      catch (Exception& e)
      {
        throw Exception(e + " in element \"row\"");
      }
    }
  }
}

/* ---------------------------------------------------------------- */

void
ApiCharSheet::find_implant_bonus (xmlNodePtr node, char const* name, double& v)
{
  if (node->type == XML_ELEMENT_NODE
      && !xmlStrcmp(node->name, (xmlChar const*)name))
  {
    node = node->children;
    while (node != 0)
    {
      this->set_double_if_node_text(node, "augmentatorValue", v);
      node = node->next;
    }
  }
}

/* ---------------------------------------------------------------- */

void
ApiCharSheet::add_char_skill (int skill_id, int level)
{
  /* Bail out if we get an invalid level. */
  if (level < 0 || level > 5)
    return;

  ApiCharSheetSkill* cskill = this->get_skill_for_id(skill_id);

  /* Bail out if the character has the skill and level. */
  if (cskill != 0 && cskill->level >= level)
    return;

  /* Create skill if the character does not have it, update otherwise. */
  if (cskill == 0)
  {
    /* Create new skill. */
    ApiSkillTreePtr tree = ApiSkillTree::request();
    ApiSkill const* skill = tree->get_skill_for_id(skill_id);

    if (skill == 0)
    {
      std::cout << "Warning: Cannot add skill (ID " << skill_id
          << ") to " << this->name << ", skill not available!" << std::endl;
      return;
    }

    int skill_start_sp = ApiCharSheet::calc_start_sp(level, skill->rank);
    int skill_dest_sp = ApiCharSheet::calc_dest_sp(level, skill->rank);
    int skill_max_points = ApiCharSheet::calc_dest_sp(4, skill->rank);

    ApiCharSheetSkill new_cskill;
    new_cskill.id = skill->id;
    new_cskill.level = level;
    new_cskill.points = skill_start_sp;
    new_cskill.points_max = skill_max_points;
    new_cskill.points_start = skill_start_sp;
    new_cskill.points_dest = skill_dest_sp;
    new_cskill.completed = 0.0;
    new_cskill.details = skill;

    this->skills.push_back(new_cskill);
    this->skills_at[level] += 1;
    this->total_sp += skill_start_sp;
  }
  else
  {
    /* Update existing skill. */
    ApiSkill const* skill = cskill->details;

    int old_level = cskill->level;
    int old_points = cskill->points;

    int skill_start_sp = ApiCharSheet::calc_start_sp(level, skill->rank);
    int skill_dest_sp = ApiCharSheet::calc_dest_sp(level, skill->rank);
    int skill_max_points = ApiCharSheet::calc_dest_sp(4, skill->rank);

    cskill->level = level;
    cskill->points = skill_start_sp;
    cskill->points_max = skill_max_points;
    cskill->points_start = skill_start_sp;
    cskill->points_dest = skill_dest_sp;
    cskill->completed = 0.0;

    this->skills_at[old_level] -= 1;
    this->skills_at[level] += 1;
    this->total_sp -= old_points;
    this->total_sp += skill_start_sp;
  }
}

/* ---------------------------------------------------------------- */

int
ApiCharSheet::get_level_for_skill (int id) const
{
  for (std::size_t i = 0; i < this->skills.size(); ++i)
    if (this->skills[i].id == id)
      return this->skills[i].level;

  /* Return level 0 if skill is not in the list. */
  return 0;
}

/* ---------------------------------------------------------------- */

ApiCharSheetSkill*
ApiCharSheet::get_skill_for_id (int id)
{
  for (std::size_t i = 0; i < this->skills.size(); ++i)
    if (this->skills[i].id == id)
      return &this->skills[i];

  return 0;
}

/* ---------------------------------------------------------------- */

ApiCharSheetCert*
ApiCharSheet::get_cert_for_id (int id)
{
  for (std::size_t i = 0; i < this->certs.size(); ++i)
    if (this->certs[i].id == id)
      return &this->certs[i];

  return 0;
}

/* ---------------------------------------------------------------- */

int
ApiCharSheet::get_grade_for_class (int class_id) const
{
  int grade = 0;
  for (std::size_t i = 0; i < this->certs.size(); ++i)
  {
    ApiCert const* cert = this->certs[i].details;
    ApiCertClass const* cclass = cert->class_details;
    if (cclass->id == class_id && cert->grade > grade)
      grade = this->certs[i].details->grade;
  }

  return grade;
}

/* ---------------------------------------------------------------- */

unsigned int
ApiCharSheet::get_spph_for_skill (ApiSkill const* skill,
    ApiCharAttribs const& attribs)
{
  double pri;
  double sec;

  if (skill == 0)
    return 0;

  switch (skill->primary)
  {
    case API_ATTRIB_INTELLIGENCE: pri = attribs.intl; break;
    case API_ATTRIB_MEMORY:       pri = attribs.mem; break;
    case API_ATTRIB_CHARISMA:     pri = attribs.cha; break;
    case API_ATTRIB_PERCEPTION:   pri = attribs.per; break;
    case API_ATTRIB_WILLPOWER:    pri = attribs.wil; break;
    default: pri = 0.0;
  }

  switch (skill->secondary)
  {
    case API_ATTRIB_INTELLIGENCE: sec = attribs.intl; break;
    case API_ATTRIB_MEMORY:       sec = attribs.mem; break;
    case API_ATTRIB_CHARISMA:     sec = attribs.cha; break;
    case API_ATTRIB_PERCEPTION:   sec = attribs.per; break;
    case API_ATTRIB_WILLPOWER:    sec = attribs.wil; break;
    default: sec = 0.0;
  }

  double sppm = (pri + sec / 2.0);
  return (unsigned int)(sppm * 60.0 + 0.5);
}

/* ---------------------------------------------------------------- */

int
ApiCharSheet::calc_start_sp (int level, int rank)
{
  if (level <= 0)
    return 0;

  return ApiCharSheet::calc_dest_sp(level - 1, rank);
}

/* ---------------------------------------------------------------- */

int
ApiCharSheet::calc_dest_sp (int level, int rank)
{
  return (int)::ceil(250.0 * rank * ::pow(32.0, level / 2.0));
}

/* ---------------------------------------------------------------- */

void
ApiCharSheet::debug_dump (void)
{
  std::cout << "Character dump for " << this->name << std::endl;
  std::cout << "List of all skills:" << std::endl;

  for (std::size_t i = 0; i < this->skills.size(); ++i)
  {
    std::cout << "  " << this->skills[i].details->name
        << " (" << this->skills[i].id << ")" << std::endl;
  }

  std::cout << "List of all certs:" << std::endl;
  for (std::size_t i = 0; i < this->certs.size(); ++i)
  {
    std::cout << "  " << /*this->certs[i].details->name*/ "Unnamed yet"
        << " (" << this->certs[i].id << ")" << std::endl;
  }
}

/* ---------------------------------------------------------------- */

bool
ApiCharSheet::is_skill_known (int id)
{
  for (size_t i = 0; i < skills.size(); i++)
  {
    if (skills[i].id == id)
      return true;
  }
  return false;
}

/* ---------------------------------------------------------------- */

ApiCharAttribs
ApiCharAttribs::operator+ (ApiCharAttribs const& atts) const
{
  ApiCharAttribs result;
  result.cha = this->cha + atts.cha;
  result.intl = this->intl + atts.intl;
  result.mem = this->mem + atts.mem;
  result.per = this->per + atts.per;
  result.wil = this->wil + atts.wil;
  return result;
}

/* ---------------------------------------------------------------- */

ApiCharAttribs
ApiCharAttribs::operator+ (double const& value) const
{
  ApiCharAttribs result;
  result.cha = this->cha + value;
  result.intl = this->intl + value;
  result.mem = this->mem + value;
  result.per = this->per + value;
  result.wil = this->wil + value;
  return result;
}

/* ---------------------------------------------------------------- */

ApiCharAttribs&
ApiCharAttribs::operator+= (ApiCharAttribs const& atts)
{
  this->cha += atts.cha;
  this->intl += atts.intl;
  this->mem += atts.mem;
  this->per += atts.per;
  this->wil += atts.wil;
  return *this;
}

/* ---------------------------------------------------------------- */

ApiCharAttribs
ApiCharAttribs::operator- (ApiCharAttribs const& atts) const
{
  ApiCharAttribs result;
  result.cha = this->cha - atts.cha;
  result.intl = this->intl - atts.intl;
  result.mem = this->mem - atts.mem;
  result.per = this->per - atts.per;
  result.wil = this->wil - atts.wil;
  return result;
}

/* ---------------------------------------------------------------- */

ApiCharAttribs
ApiCharAttribs::operator- (double const& value) const
{
  ApiCharAttribs result;
  result.cha = this->cha - value;
  result.intl = this->intl - value;
  result.mem = this->mem - value;
  result.per = this->per - value;
  result.wil = this->wil - value;
  return result;
}

/* ---------------------------------------------------------------- */

ApiCharAttribs&
ApiCharAttribs::operator-= (ApiCharAttribs const& atts)
{
  this->cha -= atts.cha;
  this->intl -= atts.intl;
  this->mem -= atts.mem;
  this->per -= atts.per;
  this->wil -= atts.wil;
  return *this;
}

/* ---------------------------------------------------------------- */

ApiCharAttribs
ApiCharAttribs::operator* (ApiCharAttribs const& atts) const
{
  ApiCharAttribs result;
  result.cha = this->cha * atts.cha;
  result.intl = this->intl * atts.intl;
  result.mem = this->mem * atts.mem;
  result.per = this->per * atts.per;
  result.wil = this->wil * atts.wil;
  return result;
}

/* ---------------------------------------------------------------- */

ApiCharAttribs
ApiCharAttribs::operator* (double const& value) const
{
  ApiCharAttribs result;
  result.cha = this->cha * value;
  result.intl = this->intl * value;
  result.mem = this->mem * value;
  result.per = this->per * value;
  result.wil = this->wil * value;
  return result;
}

/* ---------------------------------------------------------------- */

ApiCharAttribs
ApiCharAttribs::operator/ (ApiCharAttribs const& atts) const
{
  ApiCharAttribs result;
  result.cha = this->cha / atts.cha;
  result.intl = this->intl / atts.intl;
  result.mem = this->mem / atts.mem;
  result.per = this->per / atts.per;
  result.wil = this->wil / atts.wil;
  return result;
}

/* ---------------------------------------------------------------- */

ApiCharAttribs
ApiCharAttribs::operator/ (double const& value) const
{
  ApiCharAttribs result;
  result.cha = this->cha / value;
  result.intl = this->intl / value;
  result.mem = this->mem / value;
  result.per = this->per / value;
  result.wil = this->wil / value;
  return result;
}

/* ---------------------------------------------------------------- */

ApiCharAttribs&
ApiCharAttribs::operator= (double const& value)
{
  this->cha = value;
  this->intl = value;
  this->mem = value;
  this->per = value;
  this->wil = value;
  return *this;
}
