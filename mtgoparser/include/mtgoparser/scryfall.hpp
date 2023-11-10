#pragma once

#include "mtgoparser/io.hpp"

#include <boost/outcome.hpp>
#include <boost/outcome/result.hpp>

#include <glaze/glaze.hpp>
#include <spdlog/spdlog.h>

#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace scryfall {


/**
 * @brief Prices of an item (card, booster, avatar etc.) as described by Scryfall data.
 *
 * @note The prices are optional because not all cards have all prices.
 */
struct [[nodiscard]] Prices
{
  using opt_str = std::optional<std::string>;

  [[nodiscard]] explicit Prices(opt_str _usd = std::nullopt,
    opt_str _usd_foil = std::nullopt,
    opt_str _eur = std::nullopt,
    opt_str _eur_foil = std::nullopt,
    opt_str _tix = std::nullopt)
    : usd{ std::move(_usd) }, usd_foil{ std::move(_usd_foil) }, eur{ std::move(_eur) },
      eur_foil{ std::move(_eur_foil) }, tix{ std::move(_tix) }
  {}


  [[nodiscard]] inline constexpr bool operator==(const Prices &other) const
  {
    return usd == other.usd && usd_foil == other.usd_foil && eur == other.eur && eur_foil == other.eur_foil
           && tix == other.tix;
  }
  [[nodiscard]] inline constexpr bool operator!=(const Prices &other) const { return !(*this == other); }

  std::optional<std::string> usd;
  std::optional<std::string> usd_foil;
  std::optional<std::string> eur;
  std::optional<std::string> eur_foil;
  std::optional<std::string> tix;
};

/**
 * @brief A card as described by Scryfall data.
 */
struct [[nodiscard]] Card
{
  uint32_t mtgo_id{};
  std::string name{};
  std::string released_at{};
  std::string rarity{};
  Prices prices{};

  [[nodiscard]] explicit Card(uint32_t _mtgo_id = 0,
    std::string _name = "",
    std::string _released_at = "",
    std::string _rarity = "",
    Prices _prices = scryfall::Prices{})
    : mtgo_id{ _mtgo_id }, name{ std::move(_name) }, released_at{ std::move(_released_at) },
      rarity{ std::move(_rarity) }, prices{ std::move(_prices) }
  {}

  [[nodiscard]] inline constexpr bool operator==(const Card &other) const
  {
    return mtgo_id == other.mtgo_id && name == other.name && released_at == other.released_at && rarity == other.rarity
           && prices == other.prices;
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
    "name",
    &T::name,
    "released_at",
    &T::released_at,
    "rarity",
    &T::rarity,
    "prices",
    &T::prices);
};

namespace scryfall {
namespace outcome = BOOST_OUTCOME_V2_NAMESPACE;

// about 43000 cards as of 2023-09-11
const uint32_t RESERVE_APPROX_MAX_SCRYFALL_CARDS = 50000;

using ScryfallCardVec = std::vector<scryfall::Card>;
using ErrorStr = std::string;

/**
 * @brief Decodes a file as Scryfall bulk data JSON.
 *
 * @param path_json Path to Scryfall bulk data JSON file.
 *
 * @return On success: `outcome::success(ScryfallCardVec)` - A vector of Scryfall cards.
 * @return On failure: `outcome::failure(ErrorStr)`        - A string containing an error message.
 */
[[nodiscard]] auto inline ReadJsonVector(const std::filesystem::path &path_json)
  -> outcome::result<ScryfallCardVec, ErrorStr>
{
  // Instantiate and pre-allocate map
  ScryfallCardVec scryfall_vec{};
  scryfall_vec.reserve(RESERVE_APPROX_MAX_SCRYFALL_CARDS);

  // Read file into buffer and decode to populate map
  if (auto err_code = glz::read_json(scryfall_vec, io_util::read_to_str_buf(path_json))) [[unlikely]] {
    // Return error as a string
    return outcome::failure(glz::format_error(err_code, std::string{}));
  }

  return outcome::success(scryfall_vec);
}
}// namespace scryfall