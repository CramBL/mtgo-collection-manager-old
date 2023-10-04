#pragma once

#include "mtgoparser/goatbots.hpp"
#include "mtgoparser/mtgo/card.hpp"
#include "mtgoparser/mtgo/xml.hpp"
#include "mtgoparser/scryfall.hpp"

#include <glaze/glaze.hpp>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

namespace mtgo {
class Collection
{
  // Member variables
  // TODO: Add timestamp
  std::vector<Card> cards_;

  // Memoization
  // Don't have much more than 4 billion cards users please
  std::optional<uint32_t> total_quantity_ = std::nullopt;

public:
  [[nodiscard]] explicit Collection(std::vector<Card> &&cards) noexcept : cards_{ std::move(cards) } {}
  [[nodiscard]] explicit Collection(const std::string &json_str) noexcept
    : cards_{ glz::read_json<std::vector<Card>>(json_str).value() }
  {}
  [[nodiscard]] constexpr auto Size() const noexcept -> std::size_t;
  [[nodiscard]] auto TotalCards() -> uint32_t
  {
    // The first time anything related to card quantities is needed/called this function is called to avoid doing
    // double work
    if (!this->total_quantity_.has_value()) {
      this->total_quantity_ = 0;
      for (const auto &c : this->cards_) { this->total_quantity_.value() += c.quantity_; }
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

  void FromJson(const std::string &json_str);

  [[nodiscard]] inline constexpr bool operator==(const Collection &other) const { return this->cards_ == other.cards_; }
  [[nodiscard]] inline constexpr bool operator!=(const Collection &other) const { return !(*this == other); }

private:
  // Helpers
};


constexpr auto Collection::Size() const noexcept -> std::size_t { return cards_.size(); }

void Collection::ExtractGoatbotsInfo(const goatbots::card_defs_map_t &card_defs,
  const goatbots::price_hist_map_t &price_hist) noexcept
{
  for (auto &c : cards_) {
    // Extract set, rarity, and foil from goatbots card definitions
    if (auto res = card_defs.find(c.id_); res != card_defs.end()) {
      c.set_ = res->second.cardset;
      c.rarity_ = res->second.rarity;
      c.foil_ = res->second.foil == 1;
    } else {
      spdlog::warn("Card definition key not found: ID={}", c.id_);
    }
    // Extract price from goatbots price history
    if (auto res = price_hist.find(c.id_); res != price_hist.end()) {
      c.goatbots_price_ = res->second;
    } else {
      spdlog::warn("Price history key not found: ID={}", c.id_);
    }
  }
}

void Collection::ExtractScryfallInfo(std::vector<scryfall::Card> &&scryfall_cards) noexcept
{
  // Sort scryfall cards by mtgo id
  std::sort(scryfall_cards.begin(), scryfall_cards.end(), [](const scryfall::Card &a, const scryfall::Card &b) {
    return a.mtgo_id < b.mtgo_id;
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
    // Get the ID as a uint32_t
    auto c_id = static_cast<uint32_t>(std::stoul(c.id_));
    // Skip if it is foil as scryfall API doesn't have foil prices
    if (c.foil_) { continue; }

    while (scry_it != scry_end && (*scry_it).mtgo_id <= c_id) {
      if ((*scry_it).mtgo_id == c_id && (*scry_it).prices.tix.has_value() && ((*scry_it).prices.tix.value() != "")) {
        c.scryfall_price_ = std::stod((*scry_it).prices.tix.value());
      }
      ++scry_it;
    }
  }
}

[[nodiscard]] auto Collection::ToJson() const -> std::string { return glz::write_json(cards_); }
[[nodiscard]] auto Collection::ToJsonPretty() const -> std::string
{
  std::string res{};
  glz::write<glz::opts{ .prettify = true }>(cards_, res);
  return res;
}
void Collection::FromJson(const std::string &json_str)
{

  if (auto ec = glz::read_json<std::vector<Card>>(std::ref(cards_), json_str)) {
    spdlog::error("{}", glz::format_error(ec, std::string{}));
  }
}
void Collection::Print() const
{
  for (const auto &c : cards_) {
    std::string scryfall_price{ "N/A" };
    if (c.scryfall_price_.has_value()) { scryfall_price.assign(fmt::format("{0:g}", c.scryfall_price_.value())); }

    fmt::println("{} {}: Goatbots price={}, Scryfall price={}, quantity={}, set={}, foil={}, rarity={}",
      c.id_,
      c.name_,
      c.goatbots_price_,
      scryfall_price,
      c.quantity_,
      c.set_,
      c.foil_,
      c.rarity_);
  }
}

void Collection::PrettyPrint() const
{
  fmt::println("{: <25}{: <23}{: <23}{: <11}{: <8}{: <10}{: <6}\n",
    "Name",
    "Goatbots price [tix]",
    "Scryfall price [tix]",
    "Quantity",
    "Foil",
    "Rarity",
    "Set");
  for (const auto &c : cards_) {
    std::string scryfall_price{ "N/A" };
    if (c.scryfall_price_.has_value()) { scryfall_price.assign(fmt::format("{0:g}", c.scryfall_price_.value())); }
    fmt::println("{: <25}{: <23}{: <23}{: <11}{: <8}{: <10}{: <6}",
      c.name_,
      c.goatbots_price_,
      scryfall_price,
      c.quantity_,
      c.foil_,
      c.rarity_,
      c.set_);
  }
}

}// namespace mtgo
