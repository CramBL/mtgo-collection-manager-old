#pragma once

#include "glaze/glaze.hpp"

namespace goatbots
{
   struct CardDefinition
   {
      std::string name{};
      std::string cardset{};
      std::string rarity{};
      uint8_t foil{}; // actually boolean but 0/1
   };
}

template <>
struct glz::meta<goatbots::CardDefinition>
{
   using T = goatbots::CardDefinition;
   static constexpr auto value = object("name", &T::name, "cardset", &T::cardset, "rarity", &T::rarity, "foil", &T::foil);
};