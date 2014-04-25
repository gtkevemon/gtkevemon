#include <cstdlib>
#include <fstream>
#include <iostream>
#include <vector>

#include "util/helpers.h"
#include "util/exception.h"
#include "bits/config.h"
#include "xml.h"
#include "apiskilltree.h"

#define SKILLTREE_FN "SkillTree.xml"

ApiSkillTreePtr ApiSkillTree::instance;

/* ---------------------------------------------------------------- */

ApiSkillTreePtr
ApiSkillTree::request (void)
{
  if (ApiSkillTree::instance.get() == 0)
  {
    ApiSkillTree::instance = ApiSkillTreePtr(new ApiSkillTree);
    ApiSkillTree::instance->refresh();
  }

  return ApiSkillTree::instance;
}

/* ---------------------------------------------------------------- */

ApiSkillTree::ApiSkillTree (void)
{
}

/* ---------------------------------------------------------------- */

void
ApiSkillTree::refresh (void)
{
  try
  {
    this->parse_xml(this->get_filename());
    return;
  }
  catch (Exception& e)
  {
    /* Parse error occured. Report this. */
    std::cout << std::endl << "XML error: " << e << std::endl;
  }

  std::cout << "Seeking XML: " SKILLTREE_FN " not found. EXIT!" << std::endl;
  std::exit(EXIT_FAILURE);
}

/* ---------------------------------------------------------------- */

std::string
ApiSkillTree::get_filename (void) const
{
  return Config::get_conf_dir() + "/" SKILLTREE_FN;
}

/* ---------------------------------------------------------------- */

void
ApiSkillTree::parse_xml (std::string const& filename)
{
  /* Try to read the document. */
  XmlDocumentPtr xml = XmlDocument::create_from_file(filename);
  xmlNodePtr root = xml->get_root_element();

  std::cout << "Parsing XML: " SKILLTREE_FN "... " << std::flush;

  /* Document was parsed. Reset information. */
  this->skills.clear();
  this->groups.clear();
  this->parse_eveapi_tag(root);
  std::cout << this->skills.size() << " skills." << std::endl;
}

/* ---------------------------------------------------------------- */

void
ApiSkillTree::parse_eveapi_tag (xmlNodePtr node)
{
  if (node->type != XML_ELEMENT_NODE
      || xmlStrcmp(node->name, (xmlChar const*)"eveapi"))
    throw Exception("Invalid tag. Expecting <eveapi> node");

  /* Look for the result tag. */
  for (node = node->children; node != 0; node = node->next)
  {
    if (node->type != XML_ELEMENT_NODE)
      continue;

    if (!xmlStrcmp(node->name, (xmlChar const*)"result"))
    {
      //std::cout << "Found <result> tag" << std::endl;
      this->parse_result_tag(node->children);
    }
  }
}

/* ---------------------------------------------------------------- */

void
ApiSkillTree::parse_result_tag (xmlNodePtr node)
{
  /* Look for the rowset tag. It's for the skill group rowset. */
  for (; node != 0; node = node->next)
  {
    if (node->type != XML_ELEMENT_NODE)
      continue;

    if (!xmlStrcmp(node->name, (xmlChar const*)"rowset"))
    {
      //std::cout << "Found <rowset> tag for skillgroups" << std::endl;
      this->parse_groups_rowset(node->children);
    }
  }
}

/* ---------------------------------------------------------------- */

void
ApiSkillTree::parse_groups_rowset (xmlNodePtr node)
{
  /* Look for row tags. These are for the skill groups. */
  for (; node != 0; node = node->next)
  {
    if (node->type == XML_ELEMENT_NODE
        && !xmlStrcmp(node->name, (xmlChar const*)"row"))
    {
      ApiSkillGroup group;
      group.name = this->get_property(node, "groupName");
      group.id = this->get_property_int(node, "groupID");

      //std::cout << "Inserting group: " << group.name << std::endl;
      this->groups.insert(std::make_pair(group.id, group));

      this->parse_groups_row(node->children);
    }
  }
}

/* ---------------------------------------------------------------- */

void
ApiSkillTree::parse_groups_row (xmlNodePtr node)
{
  /* Look for the rowset tag. It's forthe skills rowset. */
  for (; node != 0; node = node->next)
  {
    if (node->type == XML_ELEMENT_NODE
        && !xmlStrcmp(node->name, (xmlChar const*)"rowset"))
    {
      //std::cout << "Found <rowset> tag for skills" << std::endl;
      this->parse_skills_rowset(node->children);
    }
  }
}

/* ---------------------------------------------------------------- */

void
ApiSkillTree::parse_skills_rowset (xmlNodePtr node)
{
  /* Look for row tags. These are for the skills. */
  for (; node != 0; node = node->next)
  {
    if (node->type == XML_ELEMENT_NODE
        && !xmlStrcmp(node->name, (xmlChar const*)"row"))
    {
      ApiSkill skill;
      skill.name = this->get_property(node, "typeName");
      skill.group = this->get_property_int(node, "groupID");
      skill.id = this->get_property_int(node, "typeID");
      skill.published = this->get_property_int(node, "published");
      skill.rank = 0;
      skill.primary = API_ATTRIB_UNKNOWN;
      skill.secondary = API_ATTRIB_UNKNOWN;

      this->parse_skills_row(skill, node->children);

      //std::cout << "Inserting skill:   " << skill.name << std::endl;
      this->skills.insert(std::make_pair(skill.id, skill));
    }
  }
}

/* ---------------------------------------------------------------- */

void
ApiSkillTree::parse_skills_row (ApiSkill& skill, xmlNodePtr node)
{
  for (; node != 0; node = node->next)
  {
    if (node->type != XML_ELEMENT_NODE)
      continue;

    this->set_string_if_node_text(node, "description", skill.desc);
    this->set_int_if_node_text(node, "rank", skill.rank);

    if (!xmlStrcmp(node->name, (xmlChar const*)"rowset")
        && this->get_property(node, "name") == "requiredSkills")
      this->parse_skill_requirements(skill, node->children);

    if (!xmlStrcmp(node->name, (xmlChar const*)"requiredAttributes"))
      this->parse_skill_attribs(skill, node->children);
  }
}

/* ---------------------------------------------------------------- */

void
ApiSkillTree::parse_skill_requirements (ApiSkill& skill, xmlNodePtr node)
{
  for (; node != 0; node = node->next)
  {
    if (node->type == XML_ELEMENT_NODE)
    {
      if (!xmlStrcmp(node->name, (xmlChar const*)"row"))
      {
        int type_id = this->get_property_int(node, "typeID");
        int level = this->get_property_int(node, "skillLevel");
        skill.deps.push_back(std::make_pair(type_id, level));
      }
    }
  }
}

/* ---------------------------------------------------------------- */

void
ApiSkillTree::parse_skill_attribs (ApiSkill& skill, xmlNodePtr node)
{
  std::string primary;
  std::string secondary;

  for (; node != 0; node = node->next)
  {
    if (node->type == XML_ELEMENT_NODE)
    {
      this->set_string_if_node_text(node, "primaryAttribute", primary);
      this->set_string_if_node_text(node, "secondaryAttribute", secondary);
    }
  }

  this->set_attribute(skill.primary, primary);
  this->set_attribute(skill.secondary, secondary);
}

/* ---------------------------------------------------------------- */

void
ApiSkillTree::set_attribute (ApiAttrib& var, std::string const& str)
{
  if (str == "memory")
    var = API_ATTRIB_MEMORY;
  else if (str == "charisma")
    var = API_ATTRIB_CHARISMA;
  else if (str == "intelligence")
    var = API_ATTRIB_INTELLIGENCE;
  else if (str == "perception")
    var = API_ATTRIB_PERCEPTION;
  else if (str == "willpower")
    var = API_ATTRIB_WILLPOWER;
  else
    var = API_ATTRIB_UNKNOWN;
}

/* ---------------------------------------------------------------- */

ApiSkill const*
ApiSkillTree::get_skill_for_id (int id) const
{
  ApiSkillMap::const_iterator iter = this->skills.find(id);
  if (iter == this->skills.end())
    return 0;

  return &iter->second;
}

/* ---------------------------------------------------------------- */

ApiSkill const*
ApiSkillTree::get_skill_for_name (std::string const& name) const
{
  for (ApiSkillMap::const_iterator iter = this->skills.begin();
      iter != this->skills.end(); iter++)
  {
    if (iter->second.name == name)
      return &iter->second;
  }

  return 0;
}

/* ---------------------------------------------------------------- */

ApiSkillGroup const*
ApiSkillTree::get_group_for_id (int id) const
{
  ApiSkillGroupMap::const_iterator iter = this->groups.find(id);
  if (iter == this->groups.end())
    return 0;

  return &iter->second;
}

/* ---------------------------------------------------------------- */

char const*
ApiSkillTree::get_attrib_name (ApiAttrib const& attrib)
{
  switch (attrib)
  {
    case API_ATTRIB_INTELLIGENCE: return "Intelligence";
    case API_ATTRIB_MEMORY: return "Memory";
    case API_ATTRIB_CHARISMA: return "Charisma";
    case API_ATTRIB_PERCEPTION: return "Perception";
    case API_ATTRIB_WILLPOWER: return "Willpower";
    default: break;
  }

  return "Unknown";
}

/* ---------------------------------------------------------------- */

char const*
ApiSkillTree::get_attrib_short_name (ApiAttrib const& attrib)
{
  switch (attrib)
  {
    case API_ATTRIB_INTELLIGENCE: return "Int";
    case API_ATTRIB_MEMORY: return "Mem";
    case API_ATTRIB_CHARISMA: return "Cha";
    case API_ATTRIB_PERCEPTION: return "Per";
    case API_ATTRIB_WILLPOWER: return "Wil";
    default: break;
  }

  return "???";
}

/* ---------------------------------------------------------------- */

int
ApiSkillTree::count_total_skills() const
{
  int count = 0;
  for (ApiSkillMap::const_iterator iter = this->skills.begin();
      iter != this->skills.end(); iter++)
  {
    if(iter->second.published)
      count++;
  }
  return count;
}
/* ---------------------------------------------------------------- */

void
ApiSkill::debug (void) const
{
  std::cout << "Skill: " << this->name << " (" << this->id << ")" << std::endl
      << "  Rank: " << this->rank << ", Group: " << this->group << std::endl
      << "  Attribs: " << this->primary << " / " << this->secondary << std::endl
      << "  Deps: ";
  for (unsigned int i = 0; i < this->deps.size(); ++i)
    std::cout << this->deps[i].first << " level " << this->deps[i].second << ", ";
  std::cout << std::endl;
}
