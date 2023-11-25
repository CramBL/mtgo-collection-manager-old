#pragma once

#include "mtgoparser/mtg.hpp"
#include "mtgoparser/mtgo/card.hpp"
#include "mtgoparser/mtgo/csv.hpp"
#include "mtgoparser/util.hpp"

#include <array>
#include <cstdint>
#include <optional>
#include <span>
#include <tuple>
#include <vector>

#ifdef __llvm__
#define LLVM_ASSUME(expr) __builtin_assume(expr)
#else
#define LLVM_ASSUME(expr) ((void)0)
#endif

using opt_float_t = std::optional<float>;
using opt_uint_t = std::optional<uint16_t>;

using tup_quant_and_prices_t = std::tuple<opt_uint_t, opt_float_t, opt_float_t>;

namespace mtgo {


/// Helper struct to so that `CardHistory` can be constructed with designated initializers for the fields with matching
/// types.
struct [[nodiscard]] QuantityNameSet
{
  std::string quantity_;
  std::string name_;
  std::string set_;
};

/**
 * @brief Holds the history of a card in terms of its price and quantity.
 *
 */
struct [[nodiscard]] CardHistory
{
  uint32_t id_;
  std::string quantity_;
  std::string name_;
  std::string set_;
  mtg::Rarity rarity_;
  bool foil_;
  std::vector<tup_quant_and_prices_t> price_history_;

  [[nodiscard]] explicit CardHistory(uint32_t id,
    QuantityNameSet &&qns,
    mtg::Rarity rarity,
    bool foil,
    std::vector<tup_quant_and_prices_t> &&price_history) noexcept
    : id_(id), quantity_(qns.quantity_), name_(std::move(qns.name_)), set_(std::move(qns.set_)), rarity_(rarity),
      foil_(foil), price_history_(std::move(price_history))
  {}

  // Constructor from mtgo::Card with no unavailable price history
  [[nodiscard]] explicit CardHistory(mtgo::Card &&card) noexcept
    : id_(card.id_), quantity_(std::to_string(card.quantity_)), name_(std::move(card.name_)),
      set_(std::move(card.set_)), rarity_(card.rarity_), foil_(card.foil_),
      price_history_(std::vector<tup_quant_and_prices_t>{
        std::make_tuple(opt_uint_t{ card.quantity_ }, opt_float_t{ card.goatbots_price_ }, card.scryfall_price_) })
  {}

  // Constructor from mtgo::Card with unavailable price history
  [[nodiscard]] explicit CardHistory(mtgo::Card &&card, std::vector<tup_quant_and_prices_t> &&price_history) noexcept
    : id_(card.id_), quantity_(std::to_string(card.quantity_)), name_(std::move(card.name_)),
      set_(std::move(card.set_)), rarity_(card.rarity_), foil_(card.foil_), price_history_(std::move(price_history))
  {}

  // Move constructor
  [[nodiscard]] CardHistory(CardHistory &&other) noexcept
    : id_(other.id_), quantity_(std::move(other.quantity_)), name_(std::move(other.name_)), set_(std::move(other.set_)),
      rarity_(other.rarity_), foil_(other.foil_), price_history_(std::move(other.price_history_))
  {}

  // Delete copy && assignment constructor
  CardHistory(const CardHistory &) = delete;
  CardHistory &operator=(const CardHistory &) = delete;
};

[[nodiscard]] inline auto card_history_to_csv_row(CardHistory &&card_hist) -> std::string
{
  std::string csv_row;
  csv_row.reserve(512);

  csv_row += std::to_string(card_hist.id_);
  csv_row += ',' + card_hist.quantity_;
  csv_row += ",\"" + card_hist.name_ + '\"';
  csv_row += ',' + card_hist.set_;
  csv_row += ',' + mtg::util::rarity_to_string<mtg::util::Short>(card_hist.rarity_);

  csv_row += ',';
  // Reduce branching by using a constexpr array of strings and indexing into it with the bool value.
  csv_row += util::optimization::branchless_if(card_hist.foil_, "false", "true");

  for (auto &&[quantity, gb_price, scry_price] : card_hist.price_history_) {
    csv_row += ',';
    csv_row += quantity ? fmt::format("[{}]", quantity.value()) : "";
    csv_row += gb_price ? fmt::format("{:g}", gb_price.value()) : "-";
    csv_row += ';';
    csv_row += scry_price ? fmt::format("{:g}", scry_price.value()) : "-";
  }

  return csv_row;
}

[[nodiscard]] inline auto csv_row_to_card_history(std::string &&csv_row) -> CardHistory
{

  constexpr char delimiter = ',';

  auto sub_strs = mtgo::csv::into_substr_vec(std::move(csv_row), delimiter);

  // Parse the card id
  auto id = util::sv_to_uint<uint32_t>(sub_strs[0]);
  {
    [[maybe_unused]] bool id_has_value = id.has_value();
    assert(id_has_value);
    LLVM_ASSUME(id_has_value);
  }

  // Parse the quantity, name, and set
  auto rarity = mtg::util::rarity_from_t(sub_strs[4]);
  auto foil = sub_strs[5] == "true";

  // Parse the price history
  auto price_history = mtgo::csv::quant_and_prices_from_span(std::span(sub_strs).subspan(6));

  return CardHistory{
    id.value(), QuantityNameSet{ sub_strs[1], sub_strs[2], sub_strs[3] }, rarity, foil, std::move(price_history)
  };
}

[[nodiscard]] inline auto card_history_with_prev_unavailable(mtgo::Card &&card) noexcept -> mtgo::CardHistory
{
  std::vector<tup_quant_and_prices_t> price_history;
  price_history.reserve(1);
  price_history.emplace_back(
    std::make_tuple(opt_uint_t{ card.quantity_ }, opt_float_t{ card.goatbots_price_ }, card.scryfall_price_));
  return CardHistory{ std::move(card), std::move(price_history) };
}

[[nodiscard]] inline auto card_history_with_prev_unavailable(mtgo::Card &&card,
  std::size_t num_prev_timestamps) noexcept -> mtgo::CardHistory
{
  std::vector<tup_quant_and_prices_t> price_history;
  price_history.reserve(num_prev_timestamps + 1);
  for (std::size_t i = 0; i < num_prev_timestamps; ++i) {
    price_history.emplace_back(std::make_tuple(std::nullopt, std::nullopt, std::nullopt));
  }
  // Add the available prices
  price_history.emplace_back(
    std::make_tuple(opt_uint_t{ card.quantity_ }, opt_float_t{ card.goatbots_price_ }, card.scryfall_price_));
  return CardHistory{ std::move(card), std::move(price_history) };
}

}// namespace mtgo