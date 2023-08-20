#pragma once

#include "io.hpp"
#include <glaze/glaze.hpp>
#include <spdlog/spdlog.h>

#include <concepts>
#include <optional>
#include <string>
#include <unordered_map>

namespace goatbots {
struct CardDefinition
{
  std::string name{};
  std::string cardset{};
  std::string rarity{};
  uint8_t foil{};// actually boolean but 0/1
};

using price_hist_map_t = std::unordered_map<std::string, double>;
using card_defs_map_t = std::unordered_map<std::string, CardDefinition>;

template<class T>
concept goatbots_json = std::disjunction<std::is_same<T, price_hist_map_t>, std::is_same<T, card_defs_map_t>>::value;

template<goatbots_json T> [[nodiscard]] auto ReadJsonMap(std::filesystem::path path_json) -> std::optional<T>
{
  // Instantiate and pre-allocate map
  T json_map = {};
  json_map.reserve(80000);

  // Read file into buffer and decode to populate map
  if (auto err_code = glz::read_json(json_map, io_util::ReadToStrBuf(path_json))) {
    // Handle error
    spdlog::error("code {}: {}", err_code, glz::format_error(err_code, std::string{}));
    return std::nullopt;
  }

  return json_map;
}
}// namespace goatbots

template<> struct glz::meta<goatbots::CardDefinition>
{
  using T = goatbots::CardDefinition;
  static constexpr auto value =
    object("name", &T::name, "cardset", &T::cardset, "rarity", &T::rarity, "foil", &T::foil);
};