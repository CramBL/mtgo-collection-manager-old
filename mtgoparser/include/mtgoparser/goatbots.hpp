#pragma once

#include "mtgoparser/io.hpp"

#include <boost/unordered/unordered_flat_map.hpp>
#include <glaze/glaze.hpp>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <concepts>
#include <cstdint>
#include <optional>
#include <string>

namespace goatbots {

// Try to only allocate once, so reserve more than card count
const uint32_t RESERVE_APPROX_CARD_COUNT = 80000;

struct CardDefinition
{
  std::string name{};
  std::string cardset{};
  std::string rarity{};
  uint8_t foil{};// actually boolean but 0/1

  [[nodiscard]] inline constexpr bool operator==(const CardDefinition &other) const
  {
    return name == other.name && cardset == other.cardset && rarity == other.rarity && foil == other.foil;
  }
  [[nodiscard]] inline constexpr bool operator!=(const CardDefinition &other) const { return !(*this == other); }
};

using price_hist_map_t = boost::unordered_flat_map<uint32_t, double>;
using card_defs_map_t = boost::unordered_flat_map<uint32_t, CardDefinition>;

template<class T>
concept goatbots_json = std::disjunction<std::is_same<T, price_hist_map_t>, std::is_same<T, card_defs_map_t>>::value;

template<goatbots_json T> [[nodiscard]] auto ReadJsonMap(const std::filesystem::path &path_json) -> std::optional<T>
{
  // Instantiate and pre-allocate map
  T json_map{};
  json_map.reserve(RESERVE_APPROX_CARD_COUNT);

  // Read file into buffer and decode to populate map
  if (auto err_code = glz::read_json(json_map, io_util::read_to_str_buf(path_json))) {
    // Handle error
    spdlog::error("{}", glz::format_error(err_code, std::string{}));
    return std::nullopt;
  }

  return json_map;
}

// Check if an MTGO set ID such as `RTR` is present in the card definitions
[[nodiscard]] auto inline set_id_in_card_defs(std::string_view mtgo_id, const goatbots::card_defs_map_t &card_defs)
  -> bool
{
  return std::any_of(
    card_defs.begin(), card_defs.end(), [&](const auto &card_def_kv) { return card_def_kv.second.cardset == mtgo_id; });
}

}// namespace goatbots

template<> struct glz::meta<goatbots::CardDefinition>
{
  using T = goatbots::CardDefinition;
  static constexpr auto value =
    object("name", &T::name, "cardset", &T::cardset, "rarity", &T::rarity, "foil", &T::foil);
};