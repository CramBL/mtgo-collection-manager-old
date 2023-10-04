#pragma once
// For general Magic: The Gathering info/utility

#include <glaze/glaze.hpp>

#include <cassert>
#include <cstdint>

namespace mtg {

enum class Rarity : uint8_t { Common, Uncommon, Rare, Mythic, Booster };

namespace util {
  template<typename T> constexpr auto rarity_from_t(T val) -> mtg::Rarity
  {
    if constexpr (std::convertible_to<T, std::string_view>) {
      if (val == "C" || val == "Common" || val == "common" || val == "COMMON") [[likely]] { return Rarity::Common; }
      if (val == "U" || val == "Uncommon" || val == "uncommon" || val == "UNCOMMON") [[likely]] {
        return Rarity::Uncommon;
      }
      if (val == "R" || val == "Rare" || val == "rare" || val == "RARE") [[unlikely]] { return Rarity::Rare; }
      if (val == "M" || val == "Mythic" || val == "mythic" || val == "MYTHIC") [[unlikely]] { return Rarity::Mythic; }
      if (val == "B" || val == "Booster" || val == "booster" || val == "BOOSTER") [[unlikely]] {
        return Rarity::Booster;
      }

    } else if constexpr (std::is_same_v<T, uint8_t>) {
      if (val >= 0 && val <= 4) { return static_cast<Rarity>(val); }
    } else {
      static_assert(std::is_integral<T>::value || std::convertible_to<T, std::string_view>,
        "T must either be an integral type or string-like");
    }
    throw std::invalid_argument("Invalid value for Rarity");
  }

  constexpr auto rarity_as_string(Rarity r) -> std::string
  {
    switch (r) {
    case Rarity::Common:
      [[likely]] return "Common";
    case Rarity::Uncommon:
      [[likely]] return "Uncommon";
    case Rarity::Rare:
      [[unlikely]] return "Rare";
    case Rarity::Mythic:
      [[unlikely]] return "Mythic";
    case Rarity::Booster:
      [[unlikely]] return "Booster";
    }
    assert(false);
    // If/when C++23 use: std::unreachable();
    return "";
  }

}// namespace util

}// namespace mtg


template<> struct glz::meta<mtg::Rarity>
{
  using enum mtg::Rarity;
  static constexpr auto value = enumerate("C", Common, "U", Uncommon, "R", Rare, "M", Mythic, "B", Booster);
};