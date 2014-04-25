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

#ifndef API_SKILL_TREE_HEADER
#define API_SKILL_TREE_HEADER

#include <vector>
#include <string>
#include <map>

#include "util/ref_ptr.h"
#include "apibase.h"

enum ApiAttrib
{
  API_ATTRIB_INTELLIGENCE,
  API_ATTRIB_MEMORY,
  API_ATTRIB_CHARISMA,
  API_ATTRIB_PERCEPTION,
  API_ATTRIB_WILLPOWER,
  API_ATTRIB_UNKNOWN
};

/* ---------------------------------------------------------------- */

struct ApiSkillGroup
{
  int id;
  std::string name;
};

/* ---------------------------------------------------------------- */

class ApiSkill : public ApiElement
{
  public:
    int id;
    int group;
    int rank;
    int published;
    std::string name;
    std::string desc;
    ApiAttrib primary;
    ApiAttrib secondary;

    std::vector<std::pair<int, int> > deps;

  public:
    ~ApiSkill (void) {}
    ApiElementType get_type (void) const;
    void debug (void) const;
};

/* ---------------------------------------------------------------- */

class ApiSkillTree;
typedef ref_ptr<ApiSkillTree> ApiSkillTreePtr;
typedef std::map<int, ApiSkill> ApiSkillMap;
typedef std::map<int, ApiSkillGroup> ApiSkillGroupMap;

class ApiSkillTree : public ApiBase
{
  private:
    static ApiSkillTreePtr instance;

  protected:
    ApiSkillTree (void);
    void parse_xml (std::string const& filename);
    void parse_eveapi_tag (xmlNodePtr node);
    void parse_result_tag (xmlNodePtr node);
    void parse_groups_rowset (xmlNodePtr node);
    void parse_groups_row (xmlNodePtr node);
    void parse_skills_rowset (xmlNodePtr node);
    void parse_skills_row (ApiSkill& skill, xmlNodePtr node);
    void parse_skill_requirements (ApiSkill& skill, xmlNodePtr node);
    void parse_skill_attribs (ApiSkill& skill, xmlNodePtr node);

    void set_attribute (ApiAttrib& var, std::string const& str);

  public:
    std::string filename;
    ApiSkillMap skills;
    ApiSkillGroupMap groups;

  public:
    static ApiSkillTreePtr request (void);
    void refresh (void);
    std::string get_filename (void) const;

    int count_total_skills() const;
    ApiSkill const* get_skill_for_id (int id) const;
    ApiSkill const* get_skill_for_name (std::string const& name) const;
    ApiSkillGroup const* get_group_for_id (int id) const;

    static char const* get_attrib_name (ApiAttrib const& attrib);
    static char const* get_attrib_short_name (ApiAttrib const& attrib);
};

/* ---------------------------------------------------------------- */

inline ApiElementType
ApiSkill::get_type (void) const
{
  return API_ELEM_SKILL;
}

#endif /* API_SKILL_TREE_HEADER */
