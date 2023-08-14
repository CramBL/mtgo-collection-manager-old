#pragma once

#include "glaze/glaze.hpp"

namespace goatbots
{
   struct CardDefinition
   {
      std::string name{};
      std::string set{};
      std::string rarity{};
      bool foil{};
   };
}

template <>
struct glz::meta<goatbots::CardDefinition>
{
   using T = goatbots::CardDefinition;
   static constexpr auto value = object("name", &T::name, "set", &T::set, "rarity", &T::rarity, "foil", &T::foil);
};