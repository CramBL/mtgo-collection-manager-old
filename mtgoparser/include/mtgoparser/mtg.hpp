#pragma once
// For general Magic: The Gathering info/utility

#include "mtgoparser/util.hpp"

#include <glaze/glaze.hpp>

#include <cassert>
#include <cstdint>

namespace mtg {

enum class Rarity : uint8_t { Common, Uncommon, Rare, Mythic, Booster };

namespace util {

  template<typename T> constexpr auto rarity_from_t(T val) -> mtg::Rarity
  {
    if constexpr (std::convertible_to<T, std::string_view>) {
      if (::util::is_sv_any_of(val, "C", "Common", "common", "COMMON")) [[likely]] { return Rarity::Common; }
      if (::util::is_sv_any_of(val, "U", "Uncommon", "uncommon", "UNCOMMON")) [[likely]] { return Rarity::Uncommon; }
      if (::util::is_sv_any_of(val, "R", "Rare", "rare", "RARE")) { return Rarity::Rare; }
      if (::util::is_sv_any_of(val, "M", "Mythic", "mythic", "MYTHIC")) [[unlikely]] { return Rarity::Mythic; }
      if (::util::is_sv_any_of(val, "B", "Booster", "booster", "BOOSTER")) [[unlikely]] { return Rarity::Booster; }

    } else if constexpr (std::is_same_v<T, uint8_t>) {
      if (val >= 0 && val <= 4) { return static_cast<Rarity>(val); }
    } else {
      static_assert(std::is_integral<T>::value || std::convertible_to<T, std::string_view>,
        "T must either be an integral type or string-like");
    }
    assert(false);
    // If/when C++23 use: std::unreachable();
    return Rarity::Booster;
  }

  auto inline rarity_as_string(Rarity rarity) -> std::string
  {
    switch (rarity) {
    case Rarity::Common:
      [[likely]] return "Common";
    case Rarity::Uncommon:
      [[likely]] return "Uncommon";
    case Rarity::Rare:
      return "Rare";
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