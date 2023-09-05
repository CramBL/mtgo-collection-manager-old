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
};

struct Card
{
  uint32_t mtgo_id{};
  uint32_t mtgo_foil_id{};
  std::string name{};
  std::string released_at{};
  std::string rarity{};
  Prices prices{};
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