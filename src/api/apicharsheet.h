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

#ifndef API_CHAR_SHEET_HEADER
#define API_CHAR_SHEET_HEADER

#include <string>
#include <vector>
#include <libxml/parser.h>

#include "util/ref_ptr.h"
#include "net/http.h"
#include "apibase.h"
#include "apiskilltree.h"
#include "apicerttree.h"

/* The minimum amount of seconds the sheet is cached. */
#define API_CHAR_SHEET_MIN_CACHE_TIME 1800

struct ApiCharSheetSkill
{
  int id;
  int level;
  int points;
  int points_max;
  int points_start;
  int points_dest;
  double completed;
  ApiSkill const* details;
};

/* ---------------------------------------------------------------- */

struct ApiCharSheetCert
{
  int id;
  ApiCert const* details;
};

/* ---------------------------------------------------------------- */

struct ApiCharAttribs
{
  double intl;
  double mem;
  double cha;
  double per;
  double wil;

  ApiCharAttribs (void);
  ApiCharAttribs (double value);

  ApiCharAttribs operator+ (ApiCharAttribs const& atts) const;
  ApiCharAttribs operator+ (double const& value) const;
  ApiCharAttribs& operator+= (ApiCharAttribs const& atts);

  ApiCharAttribs operator- (ApiCharAttribs const& atts) const;
  ApiCharAttribs operator- (double const& value) const;
  ApiCharAttribs& operator-= (ApiCharAttribs const& atts);

  ApiCharAttribs operator* (ApiCharAttribs const& atts) const;
  ApiCharAttribs operator* (double const& value) const;

  ApiCharAttribs operator/ (ApiCharAttribs const& atts) const;
  ApiCharAttribs operator/ (double const& value) const;

  ApiCharAttribs& operator= (double const& value);
};

/* ---------------------------------------------------------------- */

class ApiCharSheet;
typedef ref_ptr<ApiCharSheet> ApiCharSheetPtr;

class ApiCharSheet : public ApiBase
{
  /* Some internal stuff. */
  protected:
    ApiCharSheet (void);

    void parse_xml (void);
    void parse_eveapi_tag (xmlNodePtr node);
    void parse_result_tag (xmlNodePtr node);
    void parse_attribute_tag (xmlNodePtr node);
    void parse_attrib_enhancers_tag (xmlNodePtr node);
    void parse_skills_tag (xmlNodePtr node);
    void parse_certificates_tag (xmlNodePtr node);

    void find_implant_bonus (xmlNodePtr node, char const* name, double& var);
    void debug_dump (void);

  /* Publicly available collection of gathered data. */
  public:
    bool valid;

    /* Basic char information. */
    std::string char_id;
    std::string name;
    std::string race;
    std::string bloodline;
    std::string gender;
    std::string corp;
    std::string balance;

    /* Clone and respec info. */
    std::string clone_name;
    unsigned int clone_sp;
    unsigned int free_sp;
    std::string last_respec;
    std::string last_timed_respec;
    unsigned int free_respecs;
    std::string last_clone_jump;

    /* Attribute values for the character. */
    ApiCharAttribs base;
    ApiCharAttribs implant;
    ApiCharAttribs total;

    /* The vector of all known skills and certs. */
    std::vector<ApiCharSheetSkill> skills;
    std::vector<ApiCharSheetCert> certs;

    /* Statistics. */
    unsigned int total_sp;
    unsigned int skills_at[6];

  public:
    static ApiCharSheetPtr create (void);
    void set_api_data (EveApiData const& data);

    /* Check whether the character knows this skill */
    bool is_skill_known (int id);

    /* Lookup methods for skills. This is expensive. */
    ApiCharSheetSkill* get_skill_for_id (int id);
    int get_level_for_skill (int id) const;

    /* Lookup methods for certificates. This is expensive. */
    ApiCharSheetCert* get_cert_for_id (int id);
    int get_grade_for_class (int class_id) const;

    /* Adds a new skill to the character if it's not already
     * present. Statistics are appropriately updated. */
    void add_char_skill (int skill_id, int level);

    /* Calculates the SP/h for the given skill based on the
     * character total attributes or the specified attributes. */
    unsigned int get_spph_for_skill (ApiSkill const* skill);
    unsigned int get_spph_for_skill (ApiSkill const* skill,
        ApiCharAttribs const& attribs);

    /* Generic calculation of skill start and destination SP.
     * This is character independent. */
    static int calc_start_sp (int level, int rank);
    static int calc_dest_sp (int level, int rank);
};

/* ---------------------------------------------------------------- */

inline
ApiCharAttribs::ApiCharAttribs (void)
{
}

inline
ApiCharAttribs::ApiCharAttribs (double value)
{
  this->intl = value;
  this->mem = value;
  this->cha = value;
  this->per = value;
  this->wil = value;
}

inline
ApiCharSheet::ApiCharSheet (void) : valid(false)
{
}

inline ApiCharSheetPtr
ApiCharSheet::create (void)
{
  return ApiCharSheetPtr(new ApiCharSheet);
}

inline unsigned int
ApiCharSheet::get_spph_for_skill (ApiSkill const* skill)
{
  return this->get_spph_for_skill(skill, this->total);
}

#endif /* API_CHAR_SHEET_HEADER */
