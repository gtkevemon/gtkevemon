#include "implant.h"
#include "bits/config.h"
#include <iostream>
#include <cstdlib>
#include "util/helpers.h"

#define IMPLANTS_FN "implants.xml"

ImplantsPtr Implants::instance;

std::string
Implants::get_filename (void) const
{
  return Config::get_conf_dir() + "/" IMPLANTS_FN;
}

ImplantsPtr
Implants::request (void)
{
  if (Implants::instance.get() == 0)
  {
    Implants::instance = ImplantsPtr(new Implants);
    Implants::instance->refresh();
  }

  return Implants::instance;
}

void
Implants::parse_xml (std::string const& filename)
{
  // cache into data
  Helpers::read_file(filename, &data);
  /* Try to read the document. */
  XmlDocumentPtr xml = XmlDocument::create(data);
  xmlNodePtr root = xml->get_root_element();

  std::cout << "Parsing XML: " IMPLANTS_FN "... ";
  std::cout.flush();

  /* Document was parsed. Reset information. */
  this->implants.clear();

  this->parse_implants_tag(root);
  std::cout << this->implants.size() << " implants." << std::endl;
  valid = true;
}

void
Implants::parse_implants_tag (xmlNodePtr node)
{
  if (node->type != XML_ELEMENT_NODE
      || xmlStrcmp(node->name, (xmlChar const*)"implants"))
    throw Exception("Invalid XML root. Expecting <implants> node.");

  for (node = node->children; node != 0; node = node->next)
  {
    if (node->type == XML_ELEMENT_NODE
        && !xmlStrcmp(node->name, (xmlChar const*)"implant"))
    {
      this->parse_implant_tag(node);
    }
  }
}

void
Implants::parse_implant_tag (xmlNodePtr node)
{
  std::string name = this->get_property(node, "name");
  int typeID = this->get_property_int(node, "typeID");
  int charismaBonus = this->get_property_int(node, "charismaBonus");
  int intelligenceBonus = this->get_property_int(node, "intelligenceBonus");
  int memoryBonus = this->get_property_int(node, "memoryBonus");
  int willpowerBonus = this->get_property_int(node, "willpowerBonus");
  int perceptionBonus = this->get_property_int(node, "perceptionBonus");
  this->implants[typeID] = Implant(typeID, name, charismaBonus, intelligenceBonus,
                             memoryBonus, perceptionBonus, willpowerBonus);
}

void
Implants::refresh (void)
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

  std::cout << "Seeking XML: " IMPLANTS_FN " not found. EXIT!" << std::endl;
  std::exit(EXIT_FAILURE);
}

Implants::Implants (void) : valid(false)
{
}

const Implant *
Implants::getImplant(int typeID) const
{
  ImplantMap::const_iterator it = this->implants.find(typeID);
  if(it == this->implants.end())
    return 0;
  else
    return &it->second;
}

const char *Implants::get_data() const
{
  return data.c_str();
}
