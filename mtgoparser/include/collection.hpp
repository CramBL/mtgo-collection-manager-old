#pragma once
#include "goatbots.hpp"
#include "mtgo.hpp"

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
  [[nodiscard]] constexpr auto Size() const noexcept -> std::size_t;
  void ExtractGoatbotsInfo(const goatbots::card_defs_map_t &card_defs, const goatbots::price_hist_map_t &price_hist);


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

constexpr auto mtgo::Collection::Size() const noexcept -> std::size_t { return cards_.size(); }
void Collection::ExtractGoatbotsInfo(const goatbots::card_defs_map_t &card_defs,
  const goatbots::price_hist_map_t &price_hist)
{
  for (auto &c : cards_) {
    // Extract set, rarity, and foil from goatbots card definitions
    decltype(auto) card_def = card_defs.at(c.id_);

    c.set_ = card_def.cardset;
    c.rarity_ = card_def.rarity;
    c.foil_ = card_def.foil == 1;

    // Extract price from goatbots price history
    c.price_ = price_hist.at(c.id_);
  }
}

}// namespace mtgo
