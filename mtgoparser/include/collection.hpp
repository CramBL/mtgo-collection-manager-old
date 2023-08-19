#pragma once
#include "mtgo.hpp"

#include <vector>

namespace mtgo {
class Collection
{
public:
  [[nodiscard]] explicit Collection(std::vector<Card> &&cards) : cards_{ cards } {}
  [[nodiscard]] constexpr auto Size() const noexcept -> std::size_t;


private:
  [[nodiscard]] constexpr auto calc_total_card_quantity() const -> int
  {
    int total = 0;
    for (const auto &c : cards_) {
      // TODO: Parse quantity to ints and sum
      throw "Not yet implemented";
    }
    return total;
  }

  // TODO: Add timestamp
  std::vector<Card> cards_;
};

constexpr auto mtgo::Collection::Size() const noexcept -> std::size_t { return cards_.size(); }


}// namespace mtgo
