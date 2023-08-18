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
  // TODO: Add timestamp
  std::vector<Card> cards_;
};

constexpr auto mtgo::Collection::Size() const noexcept -> std::size_t { return cards_.size(); }


}// namespace mtgo
