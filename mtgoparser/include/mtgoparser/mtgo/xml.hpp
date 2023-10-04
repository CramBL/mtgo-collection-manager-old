#pragma once

#include "mtgoparser/io.hpp"
#include "mtgoparser/mtgo/card.hpp"
#include "mtgoparser/util.hpp"

#include <rapidxml/rapidxml.hpp>
#include <rapidxml/rapidxml_utils.hpp>
#include <spdlog/spdlog.h>

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace mtgo {

namespace xml {

  // TODO: Refactor and add logging
  [[nodiscard]] inline auto card_from_xml(rapidxml::xml_node<> *card_node) noexcept -> std::optional<Card>
  {
    if (!card_node) { return std::nullopt; }

    decltype(auto) first_attr = card_node->first_attribute();
    // 1st attribute
    auto id = first_attr->value();

    // 2nd attribute
    auto second_attr = first_attr->next_attribute();
    if (!second_attr) { return std::nullopt; }
    auto quantity = util::sv_to_uint<uint16_t>(second_attr->value());
    if (!quantity.has_value()) { return std::nullopt; }
    // 3rd attribute
    auto third_attr = second_attr->next_attribute();
    if (!third_attr) { return std::nullopt; }

    // 4th attribute
    auto fourth_attr = third_attr->next_attribute();
    if (!fourth_attr) { return std::nullopt; }
    auto name = fourth_attr->value();
    // 5th attribute (seems useless)
    // auto annotation = first_attr->next_attribute()->next_attribute()->next_attribute()->next_attribute()->value();

    return Card{ id, quantity.value(), name };
  }

  [[nodiscard]] auto parse_dek_xml(std::filesystem::path path_xml) noexcept -> std::vector<Card>
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
      if (auto c = card_from_xml(card)) {
        cards.emplace_back(std::move(c.value()));
      } else {
        spdlog::error("Decoding card from XML failed");
      }
    }

    return cards;
  }


}// namespace xml

}// namespace mtgo