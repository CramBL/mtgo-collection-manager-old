#pragma once

#include "io.hpp"

#include <rapidxml/rapidxml.hpp>
#include <rapidxml/rapidxml_utils.hpp>

namespace mtgo {
void parse_dek_xml(std::filesystem::path path_xml)
{
  auto buf = io_util::ReadFile(path_xml);
  rapidxml::xml_document<> doc;
  doc.parse<0>(reinterpret_cast<char *>(&buf));
}

}// namespace mtgo