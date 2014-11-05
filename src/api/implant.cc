#include "implant.h"
#include "bits/config.h"
#include <iostream>
#include <cstdlib>

#define IMPLANTS_FN "implants.xml"

ImplantsPtr Implants::instance;

static
ImplantMap implants;

void
readImplantData() {
  implants[10226] = Implant(10226,"X",5,0,0,0,0);
  implants[10208] = Implant(10208,"X",0,0,4,0,0);
  implants[10216] = Implant(10216,"X",0,0,0,4,0);
  implants[10221] = Implant(10221,"X",0,4,0,0,0);
  implants[10213] = Implant(10213,"X",0,0,0,0,5);
}

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
Implants::refresh (void)
{
  try
  {
    //    this->parse_xml(this->get_filename());
    readImplantData();
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

Implants::Implants (void)
{
}
const Implant *
Implants::getImplant(int typeID) const
{
  ImplantMap::iterator it = implants.find(typeID);
  if(it == implants.end())
    return 0;
  else
    return &it->second;
}
