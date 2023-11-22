#pragma once
// For general Magic: The Gathering info/utility

#include "mtgoparser/util.hpp"

#include <boost/implicit_cast.hpp>

#include <glaze/glaze.hpp>

#include <cassert>
#include <cstdint>
#include <type_traits>

namespace mtg {

// Denote the rarity of an MTG item.
enum class [[nodiscard]] Rarity : uint8_t{ Common, Uncommon, Rare, Mythic, Booster };

namespace util {

  /**
   * @brief Convert a string-like value to a Rarity enum.
   *
   * @tparam T The type of the string-like value
   * @param val the string-like value to convert
   * @return `mtg::Rarity` enum
   */
  template<typename T> constexpr auto rarity_from_t(T val) -> mtg::Rarity
  {
    if constexpr (std::convertible_to<T, std::string_view>) {
      if (::util::is_sv_any_of(val, "C", "Common", "common", "COMMON")) [[likely]] { return Rarity::Common; }
      if (::util::is_sv_any_of(val, "U", "Uncommon", "uncommon", "UNCOMMON")) [[likely]] { return Rarity::Uncommon; }
      if (::util::is_sv_any_of(val, "R", "Rare", "rare", "RARE")) { return Rarity::Rare; }
      if (::util::is_sv_any_of(val, "M", "Mythic", "mythic", "MYTHIC")) [[unlikely]] { return Rarity::Mythic; }
      if (::util::is_sv_any_of(val, "B", "Booster", "booster", "BOOSTER")) [[unlikely]] { return Rarity::Booster; }

    } else if constexpr (std::is_same_v<T, uint8_t>) {
      if (val >= 0 && val <= 4) { return boost::implicit_cast<Rarity>(val); }
    } else {
      static_assert(std::is_integral<T>::value || std::convertible_to<T, std::string_view>,
        "T must either be an integral type or string-like");
    }
    assert(false);
    // If/when C++23 use: std::unreachable();
    return Rarity::Booster;
  }


  // Tags for choosing format for the rarity_as_string function
  struct Short;// C, U, R, M, B
  struct Full;// Common, Uncommon, Rare, Mythic, Booster

  /**
   * @brief Convert a Rarity enum to a string representation in the specified format (`Short` or `Full`).
   *
   * @param rarity The Rarity enum to convert to a string.
   * @tparam Format The format to use for the string representation (`Short` or `Full`).
   *
   * @note The `Short` format uses the first letter of the rarity (e.g. `Rarity::Common` -> "C").
   * The `Full` format uses the full name of the rarity (e.g. `Rarity::Common` -> "Common").
   *
   * @return std::string representation of the Rarity enum.
   */
  template<typename Format>
  requires ::util::mp::is_t_any<Format, Short, Full>
  constexpr auto inline rarity_to_string(Rarity rarity) -> std::string
  {
    using ::util::mp::is_t_any;
    if constexpr (is_t_any<Format, Short>) {
      switch (rarity) {
      case Rarity::Common:
        [[likely]] return "C";
      case Rarity::Uncommon:
        [[likely]] return "U";
      case Rarity::Rare:
        return "R";
      case Rarity::Mythic:
        [[unlikely]] return "M";
      case Rarity::Booster:
        [[unlikely]] return "B";
      }
    } else if constexpr (is_t_any<Format, Full>) {
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
    } else {
      static_assert(is_t_any<Format, Short, Full>, "Format must be either mtg::util::Short or mtg::util::Full");
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