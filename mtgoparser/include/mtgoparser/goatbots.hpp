#pragma once

#include "mtgoparser/io.hpp"

#include <boost/implicit_cast.hpp>
#include <boost/outcome.hpp>
#include <boost/outcome/result.hpp>
#include <boost/unordered/unordered_flat_map.hpp>
#include <glaze/glaze.hpp>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <concepts>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace goatbots {

namespace outcome = BOOST_OUTCOME_V2_NAMESPACE;
using ErrorStr = std::string;

// Try to only allocate once, so reserve more than card count
const uint32_t RESERVE_APPROX_CARD_COUNT = 80000;

struct [[nodiscard]] CardDefinition
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

using price_hist_map_t = boost::unordered_flat_map<uint32_t, float>;
using card_defs_map_t = boost::unordered_flat_map<uint32_t, CardDefinition>;

template<class T>
concept goatbots_json = std::disjunction<std::is_same<T, price_hist_map_t>, std::is_same<T, card_defs_map_t>>::value;

template<goatbots_json T>
[[nodiscard]] auto ReadJsonMap(const std::filesystem::path &path_json) -> outcome::result<T, ErrorStr>
{
  // Instantiate and pre-allocate map
  T json_map{};
  json_map.reserve(RESERVE_APPROX_CARD_COUNT);

  // Read file into buffer and decode to populate map
  if (auto err_code = glz::read_json(json_map, io_util::read_to_str_buf(path_json))) [[unlikely]] {
    return outcome::failure(fmt::format(
      "Reading JSON from {} failed with {}", path_json.string(), glz::format_error(err_code, std::string{})));
  }

  return outcome::success(std::move(json_map));
}

// Check if an MTGO set ID such as `RTR` is present in the card definitions
[[nodiscard]] auto inline set_id_in_card_defs(std::string_view mtgo_id, const goatbots::card_defs_map_t &card_defs)
  -> bool
{
  // Make a string copy of the string_view
  std::string id_str{ mtgo_id };
  // Convert to uppercase before doing the search
  std::transform(id_str.begin(), id_str.end(), id_str.begin(), [](char chr) -> char {
    // ASCII specific, should be 100% safe for mtgo set IDs as they are IDs for software already
    if (chr >= 'a' && chr <= 'z') {
      constexpr char c_sub_to_upper = ('a' - 'A');
      return boost::implicit_cast<char>(chr - c_sub_to_upper);
    }
    return chr;
  });


  return std::any_of(
    card_defs.begin(), card_defs.end(), [&](const auto &card_def_kv) { return card_def_kv.second.cardset == id_str; });
}

}// namespace goatbots

template<> struct glz::meta<goatbots::CardDefinition>
{
  using T = goatbots::CardDefinition;
  static constexpr auto value =
    object("name", &T::name, "cardset", &T::cardset, "rarity", &T::rarity, "foil", &T::foil);
};