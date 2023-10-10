#pragma once

#include "mtgoparser/io.hpp"
#include "mtgoparser/mtgo/card.hpp"
#include "mtgoparser/util.hpp"

#include <cstddef>
#include <rapidxml/rapidxml.hpp>
#include <rapidxml/rapidxml_utils.hpp>
#include <spdlog/spdlog.h>

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace mtgo::xml {

// TODO: Refactor and add logging
[[nodiscard]] inline auto card_from_xml(rapidxml::xml_node<> *card_node) noexcept -> std::optional<Card>
{
  if (card_node == nullptr) { return std::nullopt; }

  auto *first_attr = card_node->first_attribute();
  // 1st attribute
  auto attr_id = util::sv_to_uint<uint32_t>(first_attr->value());
  if (!attr_id.has_value()) { return std::nullopt; }

  // 2nd attribute
  auto *second_attr = first_attr->next_attribute();
  if (second_attr == nullptr) { return std::nullopt; }
  auto quantity = util::sv_to_uint<uint16_t>(second_attr->value());
  if (!quantity.has_value()) { return std::nullopt; }
  // 3rd attribute
  auto *third_attr = second_attr->next_attribute();
  if (third_attr == nullptr) { return std::nullopt; }

  // 4th attribute
  auto *fourth_attr = third_attr->next_attribute();
  if (fourth_attr == nullptr) { return std::nullopt; }
  auto *name = fourth_attr->value();
  // 5th attribute (seems useless)
  // auto annotation = first_attr->next_attribute()->next_attribute()->next_attribute()->next_attribute()->value();

  return Card{ attr_id.value(), quantity.value(), name };
}

// Reserve a ballpack estimate
const std::size_t APPROX_DECK_CARDS = 1024;

[[nodiscard]] auto inline parse_dek_xml(const std::filesystem::path &path_xml) noexcept -> std::vector<Card>
{
  std::vector<char> buf = io_util::ReadToCharBuf(path_xml);
  rapidxml::xml_document<> doc;
  doc.parse<0>(buf.data());

  rapidxml::xml_node<> *first_node_ptr = doc.first_node();// `Deck` node
  // first_node() goes to `NetDeckID`
  // next_sibling() goes to `PreconstructedDeckID`
  // next_sibling goes to first `Cards` node
  auto *first_card_node = first_node_ptr->first_node()->next_sibling()->next_sibling();

  std::vector<Card> cards{};
  cards.reserve(APPROX_DECK_CARDS);

  // Iterate through all siblings
  for (auto *node = first_card_node; node != nullptr; node = node->next_sibling()) {
    // Iterate through all attributes
    if (auto card_inst = card_from_xml(node)) {
      cards.emplace_back(std::move(card_inst.value()));
    } else {
      spdlog::error("Decoding card from XML failed");
    }
  }

  return cards;
}


}// namespace mtgo::xml