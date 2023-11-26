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
enum class [[nodiscard]] Rarity : uint8_t { Common, Uncommon, Rare, Mythic, Booster };

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
  struct FullLower;// common, uncommon, rare, mythic, booster
  struct FullUpper;// COMMON, UNCOMMON, RARE, MYTHIC, BOOSTER

  /**
   * @brief Concept for the rarity formatter tags.
   *
   * @tparam Format The rarity formatter tag.
   */
  template<class Format>
  concept rarity_formatter = ::util::mp::is_t_any<Format, Short, Full, FullLower, FullUpper>;

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
  template<rarity_formatter Format> auto inline rarity_to_string(Rarity rarity) -> std::string
  {
    // Aliases for slightly more readability.
    using shortFormat = std::is_same<Format, Short>;
    using fullFormat = std::is_same<Format, Full>;
    using fullLowerFormat = std::is_same<Format, FullLower>;
    using fullUpperFormat = std::is_same<Format, FullUpper>;

    switch (rarity) {
    case Rarity::Common:
      [[likely]] if constexpr (shortFormat::value) { return "C"; }
      else if constexpr (fullFormat::value)
      {
        return "Common";
      }
      else if constexpr (fullLowerFormat::value)
      {
        return "common";
      }
      else if constexpr (fullUpperFormat::value)
      {
        return "COMMON";
      }

    case Rarity::Uncommon:
      [[likely]] if constexpr (shortFormat::value) { return "U"; }
      else if constexpr (fullFormat::value)
      {
        return "Uncommon";
      }
      else if constexpr (fullLowerFormat::value)
      {
        return "uncommon";
      }
      else if constexpr (fullUpperFormat::value)
      {
        return "UNCOMMON";
      }
    case Rarity::Rare:
      if constexpr (shortFormat::value) {
        return "R";
      } else if constexpr (fullFormat::value) {
        return "Rare";
      } else if constexpr (fullLowerFormat::value) {
        return "rare";
      } else if constexpr (fullUpperFormat::value) {
        return "RARE";
      }
    case Rarity::Mythic:
      [[unlikely]] if constexpr (shortFormat::value) { return "M"; }
      else if constexpr (fullFormat::value)
      {
        return "Mythic";
      }
      else if constexpr (fullLowerFormat::value)
      {
        return "mythic";
      }
      else if constexpr (fullUpperFormat::value)
      {
        return "MYTHIC";
      }
    case Rarity::Booster:
      [[unlikely]] if constexpr (shortFormat::value) { return "B"; }
      else if constexpr (fullFormat::value)
      {
        return "Booster";
      }
      else if constexpr (fullLowerFormat::value)
      {
        return "booster";
      }
      else if constexpr (fullUpperFormat::value)
      {
        return "BOOSTER";
      }
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