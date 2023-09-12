#pragma once

#include "mtgoparser/goatbots.hpp"
#include "mtgoparser/mtgo/card.hpp"
#include "mtgoparser/mtgo/xml.hpp"

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
  // Don't have much more than 60k of the same card users please
  std::optional<std::vector<uint16_t>> card_quantity_ = std::nullopt;

public:
  [[nodiscard]] explicit Collection(std::vector<Card> &&cards) noexcept : cards_{ std::move(cards) } {}
  [[nodiscard]] explicit Collection(const std::string &json_str) noexcept
    : cards_{ glz::read_json<std::vector<Card>>(json_str).value() }
  {}
  [[nodiscard]] constexpr auto Size() const noexcept -> std::size_t;
  [[nodiscard]] auto TotalCards() -> uint32_t { return calc_total_card_quantity(); }

  void ExtractGoatbotsInfo(const goatbots::card_defs_map_t &card_defs,
    const goatbots::price_hist_map_t &price_hist) noexcept;
  [[nodiscard]] auto ToJson() const -> std::string;
  [[nodiscard]] auto ToJsonPretty() const -> std::string;
  void Print() const;
  void FromJson(const std::string &json_str);


private:
  // Helpers
  [[nodiscard]] auto calc_total_card_quantity() -> uint32_t
  {

    // Return memoized value if it exists
    if (this->total_quantity_.has_value()) { return this->total_quantity_.value(); }

    // Parse quantity from string to uint32_t
    // Keep this result in memory for future calls including calls to specific card quantities
    std::vector<uint16_t> card_quantity_tmp(cards_.size(), 0);

    std::transform(
      // Building the vector of quantities is fully parallelizable but apple clang has not implemented std::execution :(
      // std::execution::par,
      this->cards_.begin(),
      this->cards_.end(),
      card_quantity_tmp.begin(),
      [](const mtgo::Card &c) -> uint16_t { return static_cast<uint16_t>(std::stoul(c.quantity_)); });

    // Then sum the quantities in parallel and store the result
    this->total_quantity_ = std::reduce(
      card_quantity_tmp.begin(), card_quantity_tmp.end(), 0, [](const auto &a, const auto &b) { return a + b; });

    // Move the vector to the member variable
    this->card_quantity_ = std::move(card_quantity_tmp);

    return this->total_quantity_.value();
  }
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
    spdlog::info("{} {}: price={}, quantity={}, set={}, foil={}, rarity={}",
      c.id_,
      c.name_,
      c.goatbots_price_,
      c.quantity_,
      c.set_,
      c.foil_,
      c.rarity_);
  }
}

}// namespace mtgo
