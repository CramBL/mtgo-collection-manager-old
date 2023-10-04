#pragma once

#include <compare>
#include <concepts>
#include <cstdint>
#include <glaze/glaze.hpp>
#include <optional>
#include <string>
#include <string_view>

namespace mtgo {

struct Card
{
  uint32_t id_;
  uint16_t quantity_;
  std::string name_;
  std::string set_;
  std::string rarity_;
  bool foil_;
  double goatbots_price_;
  std::optional<double> scryfall_price_;


  // Default constructor
  // Note: some builds raises false positives in static analysis when simply declared as `Card() = default` )
  [[nodiscard]] explicit Card(uint32_t id = 0,
    uint16_t quantity = 0,
    std::string name = "",
    std::string set = "",
    std::string rarity = "",
    bool foil = false,
    double goatbots_price = 0,
    std::optional<double> scryfall_price = {}) noexcept
    : id_{ id }, quantity_{ quantity }, name_{ name }, set_{ set }, rarity_{ rarity }, foil_{ foil },
      goatbots_price_{ goatbots_price }, scryfall_price_{ scryfall_price }
  {}

  // Partially parametrised constructor used to construct a card from MTGO .dek XML
  [[nodiscard]] explicit Card(uint32_t id,
    uint16_t quantity,
    const char *name,
    const char *set = "",
    const char *rarity = "",
    bool foil = false,
    double goatbots_price = 0,
    std::optional<double> scryfall_price = {}) noexcept
    : id_{ id }, quantity_{ quantity }, name_{ name }, set_{ set }, rarity_{ rarity }, foil_{ foil },
      goatbots_price_{ goatbots_price }, scryfall_price_{ scryfall_price }
  {}

  // SAFETY: The string_views used for construction has to outlive the constructed instance
  // Constructor with string_view beware of lifetimes
  [[nodiscard]] explicit Card(uint32_t id,
    uint16_t quantity,
    std::string_view name,
    std::string_view set,
    std::string_view rarity,
    bool foil = false,
    double goatbots_price = 0,
    std::optional<double> scryfall_price = {}) noexcept
    : id_{ id }, quantity_{ quantity }, name_{ name }, set_{ set }, rarity_{ rarity }, foil_{ foil },
      goatbots_price_{ goatbots_price }, scryfall_price_{ scryfall_price }
  {}

  // Templated constructor
  template<typename I, typename Q, typename S>
    requires std::convertible_to<I, uint32_t> && std::convertible_to<Q, uint16_t> && std::convertible_to<S, std::string>
  explicit Card(I id,
    Q quantity,
    S name,
    S set,
    S rarity,
    bool foil = false,
    double goatbots_price = 0,
    std::optional<double> scryfall_price = {}) noexcept
    : id_{ static_cast<uint32_t>(id) }, quantity_{ static_cast<uint16_t>(quantity) }, name_{ name }, set_{ set },
      rarity_{ rarity }, foil_{ foil }, goatbots_price_{ goatbots_price }, scryfall_price_{ scryfall_price }
  {}

  // Move constructor
  [[nodiscard]] Card(Card &&other) noexcept
    : id_(std::move(other.id_)), quantity_(std::move(other.quantity_)), name_(std::move(other.name_)),
      set_(std::move(other.set_)), rarity_(std::move(other.rarity_)), foil_(other.foil_),
      goatbots_price_(other.goatbots_price_), scryfall_price_(other.scryfall_price_)
  {}

  // Move assignment operator
  Card &operator=(Card &&other) noexcept
  {
    if (this != &other) {
      id_ = other.id_;
      quantity_ = other.quantity_;
      name_ = std::move(other.name_);
      set_ = std::move(other.set_);
      rarity_ = std::move(other.rarity_);
      foil_ = other.foil_;
      goatbots_price_ = other.goatbots_price_;
      scryfall_price_ = other.scryfall_price_;
    }

    return *this;
  }

  [[nodiscard]] inline constexpr bool operator==(const Card &other) const
  {
    return id_ == other.id_ && quantity_ == other.quantity_ && name_ == other.name_ && set_ == other.set_
           && rarity_ == other.rarity_ && foil_ == other.foil_ && goatbots_price_ == other.goatbots_price_
           && scryfall_price_ == other.scryfall_price_;
  }

  [[nodiscard]] inline constexpr bool operator!=(const Card &other) const { return !(*this == other); }
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
    "goatbots_price",
    &T::goatbots_price_,
    "scryfall_price",
    &T::scryfall_price_);
};