#pragma once

#include "glaze/glaze.hpp"
#include "io.hpp"

#include <optional>
#include <iostream>
#include <concepts>

namespace goatbots
{
   struct CardDefinition
   {
      std::string name{};
      std::string cardset{};
      std::string rarity{};
      uint8_t foil{}; // actually boolean but 0/1
   };

   using price_hist_map_t = std::unordered_map<std::string, double>;
   using card_defs_map_t = std::unordered_map<std::string, goatbots::CardDefinition>;

   template<class T>
   concept goatbots_json = std::disjunction<std::is_same<T, price_hist_map_t>, std::is_same<T, card_defs_map_t>>::value;

   template<goatbots_json T>
   auto ReadJsonMap(std::filesystem::path path_json) -> std::optional<T> {
      T json_map = {};
      json_map.reserve(80000);

      auto err_code = glz::read_json(json_map, io_util::ReadFile(path_json));

      if (err_code) {
         std::string descriptive_error = glz::format_error(err_code, std::string{});
         std::cout << "ERR=" << err_code << " parsing json: " << descriptive_error << '\n';
         return std::nullopt;
      }

      return json_map;
   }
}

template <>
struct glz::meta<goatbots::CardDefinition>
{
   using T = goatbots::CardDefinition;
   static constexpr auto value = object("name", &T::name, "cardset", &T::cardset, "rarity", &T::rarity, "foil", &T::foil);
};