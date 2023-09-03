#pragma once

#include <glaze/glaze.hpp>
#include <string>
#include <string_view>

namespace mtgo {
struct Card
{
  [[nodiscard]] explicit Card(const char *id,
    const char *quantity,
    const char *name,
    const char *set = "",
    const char *rarity = "",
    bool foil = false,
    double price = 0) noexcept
    : id_{ id }, quantity_{ quantity }, name_{ name }, set_{ set }, rarity_{ rarity }, foil_{ foil }, price_{ price }
  {}

  [[nodiscard]] explicit Card(std::string id = {},
    std::string quantity = {},
    std::string name = {},
    std::string set = {},
    std::string rarity = {},
    bool foil = false,
    double price = 0) noexcept
    : id_{ id }, quantity_{ quantity }, name_{ name }, set_{ set }, rarity_{ rarity }, foil_{ foil }, price_{ price }
  {}

  [[nodiscard]] constexpr explicit Card(std::string_view id, 
  std::string_view quantity, 
  std::string_view name, 
  std::string_view set, 
  std::string_view rarity, 
  bool foil,
  double price) noexcept: id_{ id }, quantity_{ quantity }, name_{ name }, set_{ set }, rarity_{ rarity }, foil_{ foil }, price_{ price } {}

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