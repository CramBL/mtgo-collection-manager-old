#pragma once

#include "mtgoparser/goatbots.hpp"
#include "mtgoparser/mtg.hpp"
#include "mtgoparser/mtgo/card.hpp"
#include "mtgoparser/mtgo/xml.hpp"
#include "mtgoparser/scryfall.hpp"

#include <fmt/core.h>
#include <glaze/glaze.hpp>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

namespace mtgo {

/**
 * @brief An MTGO Collection.
 */
class Collection
{
  // Member variables
  std::vector<Card> cards_;

  // Memoization
  // Don't have much more than 4 billion cards users please
  std::optional<uint32_t> total_quantity_ = std::nullopt;

public:
  // Constructors

  /**
   * @brief Construct a new Collection object from an owned (R-value) vector of cards (`mtgo::Card`).
   *
   * @param cards
   */
  [[nodiscard]] explicit Collection(std::vector<Card> &&cards) noexcept : cards_{ std::move(cards) } {}

  /**
   * @brief Construct a new Collection object from a JSON string containing a vector of cards (`mtgo::Card`) as JSON.
   *
   * @param json_str
   */
  [[nodiscard]] explicit Collection(const std::string &json_str) noexcept
    : cards_{ glz::read_json<std::vector<Card>>(json_str).value() }
  {}

  /**
   * @brief Get the size of the collection, i.e. the number of unique cards in the collection.
   *
   * @return `std::size_t` number of unique cards in the collection.
   */
  [[nodiscard]] constexpr auto Size() const noexcept -> std::size_t;

  /**
   * @brief Get the total number of cards in the collection.
   *
   * @note If you want to get the total number of unique cards in the collection, use `Size()`.
   *
   * @return `uint32_t` total quantity of cards in the collection.
   */
  [[nodiscard]] auto TotalCards() -> uint32_t
  {
    // The first time anything related to card quantities is needed/called this function is called to avoid doing
    // double work
    if (!this->total_quantity_.has_value()) {
      this->total_quantity_ = 0;
      for (const auto &card : this->cards_) { this->total_quantity_.value() += card.quantity_; }
    }
    // Return memoized value
    return this->total_quantity_.value();
  }

  void ExtractGoatbotsInfo(const goatbots::card_defs_map_t &card_defs,
    const goatbots::price_hist_map_t &price_hist) noexcept;

  void ExtractScryfallInfo(std::vector<scryfall::Card> &&scryfall_cards) noexcept;

  [[nodiscard]] auto ToJson() const -> std::string;

  [[nodiscard]] auto ToJsonPretty() const -> std::string;

  void Print() const;

  void PrettyPrint() const;

  [[nodiscard]] inline constexpr bool operator==(const Collection &other) const { return this->cards_ == other.cards_; }
  [[nodiscard]] inline constexpr bool operator!=(const Collection &other) const { return !(*this == other); }

private:
  // Helpers
};


constexpr auto Collection::Size() const noexcept -> std::size_t { return cards_.size(); }

void inline Collection::ExtractGoatbotsInfo(const goatbots::card_defs_map_t &card_defs,
  const goatbots::price_hist_map_t &price_hist) noexcept
{

  for (auto &card : this->cards_) {
    if (card.id_ == 1) [[unlikely]] {
      // Event Tickets have value 1 per definition
      card.goatbots_price_ = 1.0;
      continue;
    }
    // Extract set, rarity, and foil from goatbots card definitions
    if (auto res = card_defs.find(card.id_); res != card_defs.end()) {
      card.set_ = res->second.cardset;
      card.rarity_ = mtg::util::rarity_from_t(res->second.rarity);
      card.foil_ = res->second.foil == 1;
    } else [[unlikely]] {
      spdlog::warn("Card definition key not found: ID={}", card.id_);
    }
    // Extract price from goatbots price history
    if (auto res = price_hist.find(card.id_); res != price_hist.end()) {
      card.goatbots_price_ = res->second;
    } else [[unlikely]] {
      spdlog::warn("Price history key not found: ID={}", card.id_);
    }
  }
}

void inline Collection::ExtractScryfallInfo(std::vector<scryfall::Card> &&scryfall_cards) noexcept
{
  // Sort scryfall cards by mtgo id
  std::sort(scryfall_cards.begin(), scryfall_cards.end(), [](const scryfall::Card &cardA, const scryfall::Card &cardB) {
    return cardA.mtgo_id < cardB.mtgo_id;
  });
  // The cards_ member variable should already be sorted by ID

  // Iterator from the beginning and end of the vector
  auto scry_it = std::begin(scryfall_cards);
  auto scry_end = std::end(scryfall_cards);

  // Iterate over all the mtgo cards and the scryfall card info
  // If matching on the ID, assign the scryfall price
  // If mtgo card id is higher then scryfall id -> check the next scryfall card
  // If mtgo card id is lower than scryfall id -> check next mtgo card id
  // Loop until one of the collections is exhausted.
  for (auto &c : cards_) {
    // Skip if it is foil as scryfall API doesn't have foil prices
    if (c.foil_) { continue; }
    if (c.id_ == 1) [[unlikely]] {
      // Event ticket
      c.scryfall_price_ = 1.0;
      continue;
    }

    while (scry_it != scry_end && (*scry_it).mtgo_id <= c.id_) {
      if ((*scry_it).mtgo_id == c.id_ && (*scry_it).prices.tix.has_value()
          && (!(*scry_it).prices.tix.value().empty())) {
        c.scryfall_price_ = std::stod((*scry_it).prices.tix.value());
      }
      ++scry_it;
    }
  }
}

[[nodiscard]] auto inline Collection::ToJson() const -> std::string { return glz::write_json(cards_); }
[[nodiscard]] auto inline Collection::ToJsonPretty() const -> std::string
{
  std::string res{};
  glz::write<glz::opts{ .prettify = true }>(cards_, res);
  return res;
}

void inline Collection::Print() const
{
  for (const auto &card : cards_) {
    std::string scryfall_price{ "N/A" };
    if (card.scryfall_price_.has_value()) { scryfall_price.assign(fmt::format("{0:g}", card.scryfall_price_.value())); }

    fmt::println("{} {}: Goatbots price={}, Scryfall price={}, quantity={}, set={}, foil={}, rarity={}",
      card.id_,
      card.name_,
      card.goatbots_price_,
      scryfall_price,
      card.quantity_,
      card.set_,
      card.foil_,
      mtg::util::rarity_as_string<mtg::util::Full>(card.rarity_));
  }
}

void inline Collection::PrettyPrint() const
{
  fmt::println("{: <25}{: <23}{: <23}{: <11}{: <8}{: <10}{: <6}\n",
    "Name",
    "Goatbots price [tix]",
    "Scryfall price [tix]",
    "Quantity",
    "Foil",
    "Rarity",
    "Set");
  for (const auto &card : cards_) {
    std::string scryfall_price{ "N/A" };
    if (card.scryfall_price_.has_value()) { scryfall_price.assign(fmt::format("{0:g}", card.scryfall_price_.value())); }
    fmt::println("{: <25}{: <23}{: <23}{: <11}{: <8}{: <10}{: <6}",
      card.name_,
      card.goatbots_price_,
      scryfall_price,
      card.quantity_,
      card.foil_,
      mtg::util::rarity_as_string<mtg::util::Full>(card.rarity_),
      card.set_);
  }
}

}// namespace mtgo
