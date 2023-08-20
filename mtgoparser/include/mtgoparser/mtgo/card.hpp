#pragma once

#include <glaze/glaze.hpp>
#include <string>

namespace mtgo {
struct Card
{
  [[nodiscard]] explicit constexpr Card(const char *id,
    const char *quantity,
    const char *name,
    const char *set = "",
    const char *rarity = "",
    bool foil = false,
    double price = 0)
    : id_{ id }, name_{ name }, set_{ set }, quantity_{ quantity }, rarity_{ rarity }, foil_{ foil }, price_{ price }
  {}

  [[nodiscard]] explicit constexpr Card(std::string id = "",
    std::string quantity = "",
    std::string name = "",
    std::string set = "",
    std::string rarity = "",
    bool foil = false,
    double price = 0)
    : id_{ id }, name_{ name }, set_{ set }, quantity_{ quantity }, rarity_{ rarity }, foil_{ foil }, price_{ price }
  {}

  std::string id_;
  std::string quantity_;
  std::string name_;
  std::string set_;
  std::string rarity_;
  bool foil_;
  double price_;
};
}// namespace mtgo

template<> struct glz::meta<mtgo::Card>
{
  using T = mtgo::Card;
  static constexpr auto value = object("id",
    &T::id_,
    "quantity",
    &T::quantity_,
    "name",
    &T::name_,
    "set",
    &T::set_,
    "rarity",
    &T::rarity_,
    "foil",
    &T::foil_,
    "price",
    &T::price_);
};