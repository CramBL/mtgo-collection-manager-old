#pragma once

#include "io.hpp"

#include <rapidxml/rapidxml.hpp>
#include <rapidxml/rapidxml_utils.hpp>
#include <spdlog/spdlog.h>


namespace mtgo {
void parse_dek_xml(std::filesystem::path path_xml)
{
  std::vector<char> buf = io_util::ReadToCharBuf(path_xml);
  rapidxml::xml_document<> doc;
  doc.parse<0>(&buf[0]);

  rapidxml::xml_node<> *first_node_ptr = doc.first_node();// `Deck` node
  // first_node() goes to `NetDeckID`
  // next_sibling() goes to `PreconstructedDeckID`
  // next_sibling goes to first `Cards` node
  decltype(auto) first_card_node = first_node_ptr->first_node()->next_sibling()->next_sibling();

  // Iterate through all siblings
  for (decltype(auto) card = first_card_node; card; card = card->next_sibling()) {
    // Iterate through all attributes
    for (decltype(auto) attr = card->first_attribute(); attr; attr = attr->next_attribute()) {
      spdlog::info("{}={}", attr->name(), attr->value());
    }
  }
}

}// namespace mtgo