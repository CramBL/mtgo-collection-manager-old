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

  spdlog::info("First node: {} with {}", doc.first_node()->name(), doc.first_node()->first_attribute()->name());
  spdlog::info(
    "First node: {} with {}", doc.first_node()->first_node()->name(), doc.first_node()->first_attribute()->name());
  spdlog::info("First node: {} with {}",
    doc.first_node()->first_node()->first_node()->name(),
    doc.first_node()->first_attribute()->name());

  decltype(auto) start_of_card_siblings = doc.first_node();

  for (decltype(auto) card = start_of_card_siblings->first_node(); card; card->next_sibling()) {
    spdlog::info(
      "card: ID:{} Quantity={}", card->first_attribute()->name(), card->first_attribute()->next_attribute()->name());
  }
}

}// namespace mtgo