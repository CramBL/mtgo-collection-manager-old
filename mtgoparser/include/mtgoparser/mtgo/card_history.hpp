
#include "mtgoparser/mtg.hpp"

#include <cstdint>
#include <optional>
#include <tuple>
#include <vector>

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

  explicit CardHistory(uint32_t id,
    QuantityNameSet &&qns,
    mtg::Rarity rarity,
    bool foil,
    std::vector<tup_quant_and_prices_t> &&price_history) noexcept
    : id_(id), quantity_(qns.quantity_), name_(std::move(qns.name_)), set_(std::move(qns.set_)), rarity_(rarity),
      foil_(foil), price_history_(std::move(price_history))
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
  csv_row += ',';
  csv_row += card_hist.quantity_;
  csv_row += ',';
  csv_row += card_hist.name_;
  csv_row += ',';
  csv_row += card_hist.set_;
  csv_row += ',';
  csv_row += mtg::util::rarity_as_string<mtg::util::Short>(card_hist.rarity_);
  csv_row += ',';

  // Reduce branching
  constexpr std::array is_foil_str = { "false", "true" };
  csv_row += is_foil_str[boost::implicit_cast<uint8_t>(card_hist.foil_)];

  for (auto &&[quantity, gb_price, scry_price] : card_hist.price_history_) {
    csv_row += ',';
    csv_row += quantity ? fmt::format("[{}]", quantity.value()) : "";
    csv_row += gb_price ? fmt::format("{:g}", gb_price.value()) : "-";
    csv_row += ';';
    csv_row += scry_price ? fmt::format("{:g}", scry_price.value()) : "-";
  }

  return csv_row;
}

}// namespace mtgo