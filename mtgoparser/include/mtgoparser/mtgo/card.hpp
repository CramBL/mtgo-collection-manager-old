#pragma once

#include <concepts>
#include <glaze/glaze.hpp>
#include <string>
#include <string_view>

namespace mtgo {
struct Card
{
  [[nodiscard]] explicit Card() = default;

  [[nodiscard]] explicit Card(const char *id,
    const char *quantity,
    const char *name,
    const char *set = "",
    const char *rarity = "",
    bool foil = false,
    double price = 0) noexcept
    : id_{ id }, quantity_{ quantity }, name_{ name }, set_{ set }, rarity_{ rarity }, foil_{ foil }, price_{ price }
  {}


  template<typename T>
  requires std::convertible_to<T, std::string>
  explicit Card(T id, T quantity, T name, T set, T rarity = "", bool foil = false, double price = 0) noexcept
    : id_{ id }, quantity_{ quantity }, name_{ name }, set_{ set }, rarity_{ rarity }, foil_{ foil }, price_{ price }
  {}

  [[nodiscard]] Card(Card &&other) noexcept
    : id_(std::move(other.id_)), quantity_(std::move(other.quantity_)), name_(std::move(other.name_)),
      set_(std::move(other.set_)), rarity_(std::move(other.rarity_)), foil_(other.foil_), price_(other.price_)
  {}

  [[nodiscard]] Card &operator=(Card &&other) noexcept
  {
    if (this != &other) {
      id_ = std::move(other.id_);
      quantity_ = std::move(other.quantity_);
      name_ = std::move(other.name_);
      set_ = std::move(other.set_);
      rarity_ = std::move(other.rarity_);
      foil_ = other.foil_;
      price_ = other.price_;
    }

    return *this;
  }

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