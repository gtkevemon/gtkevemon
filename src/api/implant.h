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

#ifndef IMPLANT_HEADER
#define IMPLANT_HEADER

#include <string>
#include <map>
#include <libxml/parser.h>

#include "net/http.h"
#include "xml.h"
#include "util/ref_ptr.h"

class Implant
{
  public:
  int typeID;
  std::string name;
  int charismaBonus;
  int intelligenceBonus;
  int memoryBonus;
  int perceptionBonus;
  int willpowerBonus;
  Implant(const Implant &other)
    : typeID(other.typeID), name(other.name), charismaBonus(other.charismaBonus),
      intelligenceBonus(other.intelligenceBonus), memoryBonus(other.memoryBonus),
      perceptionBonus(other.perceptionBonus), willpowerBonus(other.willpowerBonus)
  {}
  Implant() {}
  Implant(int typeID, const std::string &name, int charismaBonus,
          int intelligenceBonus, int memoryBonus, int perceptionBonus,
          int willpowerBonus)
    : typeID(typeID), name(name), charismaBonus(charismaBonus),
      intelligenceBonus(intelligenceBonus), memoryBonus(memoryBonus),
      perceptionBonus(perceptionBonus), willpowerBonus(willpowerBonus)
  {}
};

typedef std::map<int, Implant> ImplantMap;

class Implants;

typedef ref_ptr<Implants> ImplantsPtr;

class Implants : public XmlBase
{
  private:
    static ImplantsPtr instance;
  protected:
    Implants();
    void parse_xml (const std::string &filename);
    void parse_implants_tag (xmlNodePtr node);
    void parse_implant_tag (xmlNodePtr node);
    ImplantMap implants;
    std::string data;
  public:
    bool valid;
    static ImplantsPtr request (void);
    void refresh (void);
    std::string get_filename (void) const;
    const Implant *getImplant(int typeID) const;
    const char *get_data() const;
};
  
#endif /* IMPLANT_HEADER */
