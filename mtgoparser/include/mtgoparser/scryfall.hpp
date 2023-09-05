#pragma once

#include "mtgoparser/io.hpp"
#include <glaze/glaze.hpp>
#include <optional>
#include <spdlog/spdlog.h>
#include <string>
#include <string_view>

namespace scryfall {
struct Prices
{
  std::optional<std::string> usd{};
  std::optional<std::string> usd_foil{};
  std::optional<std::string> eur{};
  std::optional<std::string> eur_foil{};
  std::optional<std::string> tix{};


  [[nodiscard]] inline constexpr bool operator==(const Prices &other) const
  {
    return usd == other.usd && usd_foil == other.usd_foil && eur == other.eur && eur_foil == other.eur_foil
           && tix == other.tix;
  }
  [[nodiscard]] inline constexpr bool operator!=(const Prices &other) const { return !(*this == other); }
};

struct Card
{
  uint32_t mtgo_id{};
  uint32_t mtgo_foil_id{};
  std::string name{};
  std::string released_at{};
  std::string rarity{};
  Prices prices{};

  [[nodiscard]] inline constexpr bool operator==(const Card &other) const
  {
    return mtgo_id == other.mtgo_id && mtgo_foil_id == other.mtgo_foil_id && name == other.name
           && released_at == other.released_at && rarity == other.rarity && prices == other.prices;
  }

  [[nodiscard]] inline constexpr bool operator!=(const Card &other) const { return !(*this == other); }
};
}// namespace scryfall

template<> struct glz::meta<scryfall::Prices>
{
  using T = scryfall::Prices;
  static constexpr std::string_view name = "prices";
  static constexpr auto value =
    object("usd", &T::usd, "usd_foil", &T::usd_foil, "eur", &T::eur, "eur_foil", &T::eur_foil, "tix", &T::tix);
};

template<> struct glz::meta<scryfall::Card>
{
  using T = scryfall::Card;
  static constexpr auto value = object("mtgo_id",
    &T::mtgo_id,
    "mtgo_foil_id",
    &T::mtgo_foil_id,
    "name",
    &T::name,
    "rarity",
    &T::rarity,
    "prices",
    &T::prices);
};