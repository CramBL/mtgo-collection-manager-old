#pragma once

#include "io.hpp"

#include <rapidxml/rapidxml.hpp>
#include <rapidxml/rapidxml_utils.hpp>
#include <spdlog/spdlog.h>

#include <filesystem>
#include <string>
#include <vector>


namespace mtgo {

namespace xml {
  [[nodiscard]] inline auto card_from_xml(rapidxml::xml_node<> *card_node) -> Card
  {
    decltype(auto) first_attr = card_node->first_attribute();
    // 1st attribute
    auto id = first_attr->value();
    // 2nd attribute
    auto quantity = first_attr->next_attribute()->value();
    // 4th attribute
    auto name = first_attr->next_attribute()->next_attribute()->next_attribute()->value();
    // 5th attribute (seems useless)
    // auto annotation = first_attr->next_attribute()->next_attribute()->next_attribute()->next_attribute()->value();

    return Card(id, quantity, name);
  }

  [[nodiscard]] auto parse_dek_xml(std::filesystem::path path_xml) -> std::vector<Card>
  {
    std::vector<char> buf = io_util::ReadToCharBuf(path_xml);
    rapidxml::xml_document<> doc;
    doc.parse<0>(&buf[0]);

    rapidxml::xml_node<> *first_node_ptr = doc.first_node();// `Deck` node
    // first_node() goes to `NetDeckID`
    // next_sibling() goes to `PreconstructedDeckID`
    // next_sibling goes to first `Cards` node
    decltype(auto) first_card_node = first_node_ptr->first_node()->next_sibling()->next_sibling();

    std::vector<Card> cards{};
    cards.reserve(500);

    // Iterate through all siblings
    for (decltype(auto) card = first_card_node; card; card = card->next_sibling()) {
      // Iterate through all attributes
      cards.emplace_back(card_from_xml(card));
    }

    return cards;
  }


}// namespace xml

}// namespace mtgo