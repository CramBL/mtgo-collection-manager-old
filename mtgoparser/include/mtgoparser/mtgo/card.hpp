#pragma once

#include <string>

namespace mtgo {
struct Card
{
  [[nodiscard]] explicit constexpr Card(const char *id,
    const char *quantity,
    const char *name,
    // const char *annotation,
    const char *set = "",
    const char *rarity = "",
    bool foil = false,
    double price = 0)
    : id_{ id }, name_{ name }, set_{ set }, quantity_{ quantity }, rarity_{ rarity }, foil_{ foil }, price_{ price }
      //, annotation_{ annotation }
  {}

  const std::string id_;
  std::string quantity_;
  const std::string name_;
  // std::string annotation_;// This attribute seems useless (yet to see it be not 0)
  std::string set_;
  std::string rarity_;
  bool foil_;
  double price_;
};
}// namespace mtgo