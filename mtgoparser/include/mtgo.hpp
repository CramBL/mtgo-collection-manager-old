#pragma once

#include "io.hpp"

#include <rapidxml/rapidxml.hpp>
#include <rapidxml/rapidxml_utils.hpp>
#include <spdlog/spdlog.h>


namespace mtgo {
void parse_dek_xml(std::filesystem::path path_xml)
{
  spdlog::info("Reading from {}", path_xml.string());
  std::vector<char> buf = io_util::ReadToCharBuf(path_xml);
  spdlog::info("Got {} bytes", buf.size());
  rapidxml::xml_document<> doc;
  doc.parse<0>(&buf[0]);

  spdlog::info("First node: {}", doc.first_node()->name());
}

}// namespace mtgo