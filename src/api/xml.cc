#include <cstring>
#include <cerrno>

#include "util/exception.h"
#include "util/helpers.h"
#include "xml.h"

void
XmlDocument::parse (std::string const& data)
{
  xmlFreeDoc(this->doc);

  this->doc = xmlParseMemory(data.c_str(), (int)data.size());
  if (this->doc == 0)
    throw Exception("Document not parsed successfully!");
}

/* ---------------------------------------------------------------- */

void
XmlDocument::parse (char const* data, std::size_t size)
{
  xmlFreeDoc(this->doc);

  this->doc = xmlParseMemory(data, (int)size);
  if (this->doc == 0)
    throw Exception("Document not parsed successfully!");
}

/* ---------------------------------------------------------------- */

void
XmlDocument::parse_from_file (std::string const& filename)
{
  std::string contents;
  Helpers::read_file(filename, &contents, true);
  this->parse(contents);
}

/* ---------------------------------------------------------------- */

xmlNodePtr
XmlDocument::get_root_element (void)
{
  xmlNodePtr cur = xmlDocGetRootElement(this->doc);
  if (!cur)
    throw Exception("Document has no root element!");

  return cur;
}

/* ================================================================ */

std::string
XmlBase::get_node_text (xmlNodePtr node)
{
  xmlChar* text = xmlNodeGetContent(node);
  if (text == 0)
    throw Exception("Could not retrieve error node text");
  std::string ret((char const*)text);
  xmlFree(text);
  return ret;
}

/* ---------------------------------------------------------------- */

std::string
XmlBase::get_property (xmlNodePtr node, char const* name)
{
  xmlChar* text = xmlGetProp(node, (xmlChar const*)name);
  if (text == 0)
    throw Exception(std::string("Could not find property \"") + name + "\"");
  std::string ret((char const*)text);
  xmlFree(text);
  return ret;
}

/* ---------------------------------------------------------------- */

int
XmlBase::get_property_int (xmlNodePtr node, char const* name)
{
  std::string prop_str = this->get_property(node, name);
  return Helpers::get_int_from_string(prop_str);
}

/* ---------------------------------------------------------------- */

void
XmlBase::set_string_if_node_text (xmlNodePtr node, char const* node_name,
    std::string& target)
{
  if (!xmlStrcmp(node->name, (xmlChar const*)node_name))
    target = this->get_node_text(node);
}

/* ---------------------------------------------------------------- */

void
XmlBase::set_int_if_node_text (xmlNodePtr node, char const* node_name,
    int& target)
{
  if (!xmlStrcmp(node->name, (xmlChar const*)node_name))
    target = Helpers::get_int_from_string(this->get_node_text(node));
}

/* ---------------------------------------------------------------- */

void
XmlBase::set_uint_if_node_text (xmlNodePtr node, char const* node_name,
    unsigned int& target)
{
  if (!xmlStrcmp(node->name, (xmlChar const*)node_name))
    target = Helpers::get_uint_from_string(this->get_node_text(node));
}

/* ---------------------------------------------------------------- */

void
XmlBase::set_double_if_node_text (xmlNodePtr node, char const* node_name,
    double& target)
{
  if (!xmlStrcmp(node->name, (xmlChar const*)node_name))
    target = Helpers::get_double_from_string(this->get_node_text(node));
}

/* ---------------------------------------------------------------- */

void
XmlBase::set_bool_if_node_text (xmlNodePtr node, char const* node_name,
    bool& target)
{
  if (!xmlStrcmp(node->name, (xmlChar const*)node_name))
    target = (bool)Helpers::get_int_from_string(this->get_node_text(node));
}
