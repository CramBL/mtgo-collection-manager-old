#pragma once

#include "mtgo.hpp"

#include <vector>

namespace mtgo {
class Collection
{
public:
  [[nodiscard]] explicit Collection(std::vector<Card> &&cards) : cards_{ cards } {}

private:
  // TODO: Add timestamp
  std::vector<Card> cards_;
};
}// namespace mtgo