#pragma once

#include "mtgoparser/mtgo/card.hpp"
#include "mtgoparser/mtgo/xml.hpp"

#include "mtgoparser/goatbots.hpp"
#include <glaze/glaze.hpp>
#include <spdlog/spdlog.h>
#include <string>
#include <vector>

namespace mtgo {
class Collection
{
  // Member variables
  // TODO: Add timestamp
  std::vector<Card> cards_;
  int total_quantity_ = 0;

public:
  [[nodiscard]] explicit Collection(std::vector<Card> &&cards) noexcept : cards_{ cards } {}
  [[nodiscard]] explicit Collection(const std::string &json_str) noexcept
    : cards_{ glz::read_json<std::vector<Card>>(json_str).value() }
  {}
  [[nodiscard]] constexpr auto Size() const noexcept -> std::size_t;
  void ExtractGoatbotsInfo(const goatbots::card_defs_map_t &card_defs,
    const goatbots::price_hist_map_t &price_hist) noexcept;
  [[nodiscard]] auto ToJson() const -> std::string;
  [[nodiscard]] auto ToJsonPretty() const -> std::string;
  void Print();
  void FromJson(const std::string &json_str);


private:
  // Helpers
  [[nodiscard]] constexpr auto calc_total_card_quantity() const -> int
  {
    int total = 0;
    for (const auto &c : cards_) {
      // TODO: Parse quantity to ints and sum
      throw "Not yet implemented";
    }
    return total;
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
      c.price_ = res->second;
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
  auto ec = glz::read_json<std::vector<Card>>(std::ref(cards_), json_str);
  // TODO: handle error
}
void Collection::Print()
{
  for (const auto &c : cards_) {
    spdlog::info("{} {}: price={}, quantity={}, set={}, foil={}, rarity={}",
      c.id_,
      c.name_,
      c.price_,
      c.quantity_,
      c.set_,
      c.foil_,
      c.rarity_);
  }
}

}// namespace mtgo
