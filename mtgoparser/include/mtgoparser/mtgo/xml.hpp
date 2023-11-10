#pragma once

#include "mtgoparser/io.hpp"
#include "mtgoparser/mtgo/card.hpp"
#include "mtgoparser/util.hpp"

#include <cstddef>
#include <fmt/core.h>
#include <rapidxml/rapidxml.hpp>
#include <rapidxml/rapidxml_utils.hpp>

#include <boost/outcome.hpp>
#include <boost/outcome/result.hpp>

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace mtgo::xml {

namespace outcome = BOOST_OUTCOME_V2_NAMESPACE;
using ErrorStr = std::string;

/**
 * @brief Parse a `mtgo::Card` from an XML node.
 *
 * @note The XML node should be a `Cards` node from an MTGO `Full Trade List.dek`-file.
 *
 * @param card_node
 * @return outcome::result<Card, ErrorStr> A `Card` if parsing was successful, otherwise an `ErrorStr`.
 */
[[nodiscard]] inline auto card_from_xml(rapidxml::xml_node<> *card_node) noexcept -> outcome::result<Card, ErrorStr>
{
  if (card_node == nullptr) [[unlikely]] { return outcome::failure("card_node is null"); }

  auto *first_attr = card_node->first_attribute();
  // 1st attribute (MTGO ID)
  auto res_attr_id = util::sv_to_uint<uint32_t>(first_attr->value());
  if (res_attr_id.has_error()) [[unlikely]] { return outcome::failure(res_attr_id.error()); }

  // 2nd attribute (quantity)
  auto *second_attr = first_attr->next_attribute();
  if (second_attr == nullptr) [[unlikely]] { return outcome::failure("second_attr node (should be quantity) is null"); }

  auto res_quantity = util::sv_to_uint<uint16_t>(second_attr->value());
  if (res_quantity.has_error()) [[unlikely]] { return outcome::failure(res_quantity.error()); }


  // 3rd attribute (Sideboard, we don't care about this but have to go through it)
  auto *third_attr = second_attr->next_attribute();
  if (third_attr == nullptr) [[unlikely]] { return outcome::failure("third_attr node (should be sideboard) is null"); }

  // 4th attribute (name)
  auto *fourth_attr = third_attr->next_attribute();
  if (fourth_attr == nullptr) [[unlikely]] { return outcome::failure("fourth_attr node (should be name) is null"); }
  auto *name = fourth_attr->value();
  // 5th attribute (seems useless)
  // auto annotation = first_attr->next_attribute()->next_attribute()->next_attribute()->next_attribute()->value();

  return outcome::success(Card{ res_attr_id.value(), res_quantity.value(), name });
}

// Reserve a ballpack estimate
const std::size_t APPROX_DECK_CARDS = 1024;

/**
 * @brief Parse a `std::vector<mtgo::Card>` from an XML file.
 *
 * @note The XML file should be an MTGO `Full Trade List.dek`-file.
 *
 * @param path_xml A path to the XML file.
 * @return `outcome::result<std::vector<Card>, ErrorStr>` A `std::vector<Card>` if parsing was successful, otherwise an
 * `ErrorStr`.
 */
[[nodiscard]] auto inline parse_dek_xml(const std::filesystem::path &path_xml) noexcept
  -> outcome::result<std::vector<Card>, ErrorStr>
{
  std::vector<char> buf = io_util::read_to_char_buf(path_xml);
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
    if (auto res_card_inst = card_from_xml(node); res_card_inst.has_value()) {
      cards.emplace_back(std::move(res_card_inst.value()));
    } else [[unlikely]] {
      return outcome::failure(fmt::format("Decoding card from XML failed: {}", res_card_inst.error()));
    }
  }

  return outcome::success(std::move(cards));
}


}// namespace mtgo::xml