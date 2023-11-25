#pragma once

// Adds collections together to form an aggregate collection with a history of prices etc.
// Saves the aggregate as CSV

/* The collection data JSON files are named e.g. like this:
    mtgo-cards_2023-11-08T200010Z.json
   And contains a JSON array of mtgo::Card objects.

   The steps to aggregate the collections are:
    1. Read JSON files in order of date (oldest first)
    2. Aggregate the collections in a collection aggregate class (define an add function for the class)
       - Add goatbots price history and scryfall price history to the aggregate
       - When adding a card to the aggregate, if the card already exists in the aggregate, update the
         card's price history with the new price history
       - If the card does not exist in the aggregate, add it to the aggregate
    3. Save the aggregate as a CSV file
    4. Zip all the aggregate CSV files into a single zip file
*/

#include "mtgoparser/mtgo.hpp"
#include "mtgoparser/mtgo/card_history.hpp"
#include "mtgoparser/mtgo/csv.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <string>
#include <utility>
#include <vector>

namespace mtgo {

struct [[nodiscard]] CardHistoryAggregate
{
  mtgo::CardHistory card_history_;
  // The newest quantity is the last time the quantity changed
  // Used to determine if the history should include a quantity update
  uint16_t newest_quantity_;

  // Equality operators
  [[nodiscard]] inline auto operator==(const CardHistoryAggregate &other) const noexcept -> bool
  {
    return this->card_history_ == other.card_history_ && this->newest_quantity_ == other.newest_quantity_;
  }

  [[nodiscard]] inline auto operator!=(const CardHistoryAggregate &other) const noexcept -> bool
  {
    return !(*this == other);
  }
};

class CollectionHistory
{
private:
  std::vector<std::string> timestamps_;
  std::vector<CardHistoryAggregate> card_histories_;

public:
  [[nodiscard]] explicit CollectionHistory() = default;

  [[nodiscard]] explicit CollectionHistory(std::vector<mtgo::CardHistory> &&card_histories,
    std::string &&timestamp) noexcept
  {
    for (auto &&card_history : card_histories) {
      uint16_t newest_quantity = 0;
      for (auto [quantity, price, foil_price] : card_history.price_history_) {
        if (quantity.has_value()) { newest_quantity = quantity.value(); }
      }
      this->card_histories_.emplace_back(
        CardHistoryAggregate{ .card_history_{ std::move(card_history) }, .newest_quantity_ = newest_quantity });
    }
    this->timestamps_.emplace_back(std::move(timestamp));
  }

  [[nodiscard]] explicit CollectionHistory(std::vector<CardHistoryAggregate> &&card_histories,
    std::vector<std::string> &&timestamps) noexcept
    : timestamps_(std::move(timestamps)), card_histories_(std::move(card_histories))
  {}

  void addCollectionPriceHistory(mtgo::Collection &&collection, std::string &&timestamp)
  {
    auto cards = collection.TakeCards();

    for (auto &&card : cards) {
      auto it = std::find_if(this->card_histories_.begin(), this->card_histories_.end(), [&](const auto &card_hist) {
        return card_hist.card_history_.id_ == card.id_;
      });

      if (it != card_histories_.end()) {
        if (card.quantity_ != it->newest_quantity_) {
          it->newest_quantity_ = card.quantity_;
          it->card_history_.price_history_.emplace_back(
            std::make_tuple(card.quantity_, card.goatbots_price_, card.scryfall_price_));
        } else {
          // Quantity has not changed, but price history should still be added
          it->card_history_.price_history_.emplace_back(
            std::make_tuple(std::nullopt, card.goatbots_price_, card.scryfall_price_));
        }
      } else {
        // Add the card history
        this->card_histories_.emplace_back(CardHistoryAggregate{
          .card_history_{ card_history_with_prev_unavailable(std::move(card), this->timestamps_.size()) },
          .newest_quantity_ = card.quantity_ });
      }
    }
    this->timestamps_.emplace_back(std::move(timestamp));
  }

  [[nodiscard]] inline auto Size() const noexcept -> std::size_t { return card_histories_.size(); }

  /**
   * @brief Consumes/moves the CardHistory vector and returns the collection history as a CSV string.
   *
   * @return std::string The collection history as a CSV string.
   */
  [[nodiscard]] inline auto ToCsvStr() noexcept -> std::string
  {
    std::string csv_str;
    static constexpr std::size_t prealloc_10_mib = 1024 * 1024 * 10;
    csv_str.reserve(prealloc_10_mib);

    // Write the header
    csv_str += "id,quantity,name,set,rarity,foil";
    for (auto &&timestamp : this->timestamps_) { csv_str += ',' + timestamp; }

    // Write the card histories
    for (auto &&card_hist : this->card_histories_) {
      csv_str += '\n';
      csv_str += card_history_to_csv_row(std::move(card_hist.card_history_));
    }

    return csv_str;
  }

  /**
   * @brief Saves the collection history as a CSV file with the newest timestamp as suffix.
   *
   * @note `fpath` is modified to include the newest timestamp as suffix.
   *
   * @param fpath The path to the CSV file to add the timestamp to and then save.
   */
  inline void SaveAsCsvWithTimestamp(std::filesystem::path &fpath) noexcept
  {
    // Add the the newest timestamp to the filename as suffix
    fpath.replace_filename(fpath.filename().string() + '_' + this->timestamps_.back() + ".csv");

    // Save the CSV file
    std::ofstream csv_file(fpath);
    csv_file << this->ToCsvStr();
  }

  // Comparison operators
  [[nodiscard]] inline auto operator==(const CollectionHistory &other) const noexcept -> bool
  {
    return this->timestamps_ == other.timestamps_ && this->card_histories_ == other.card_histories_;
  }

  [[nodiscard]] inline auto operator!=(const CollectionHistory &other) const noexcept -> bool
  {
    return !(*this == other);
  }
};


[[nodiscard]] inline auto csv_to_collection_history(std::string &&csv_str) -> CollectionHistory
{
  auto csv_lines = mtgo::csv::into_lines_vec(csv_str);

  // Save the timestamps from the first line
  auto timestamps = mtgo::csv::into_substr_vec(csv_lines[0], ',');
  timestamps.erase(timestamps.begin(), timestamps.begin() + 6);

  // Remove the first line
  csv_lines.erase(csv_lines.begin());

  // Parse the rest of the CSV into std::vector<CardHistoryAggregate>
  std::vector<CardHistoryAggregate> card_histories;
  card_histories.reserve(csv_lines.size());
  for (auto &&csv_line : csv_lines) {
    auto card_hist = mtgo::csv_row_to_card_history(std::move(csv_line));
    uint16_t newest_quantity = 0;
    for (auto [quantity, price, foil_price] : card_hist.price_history_) {
      if (quantity.has_value()) { newest_quantity = quantity.value(); }
    }
    card_histories.emplace_back(
      CardHistoryAggregate{ .card_history_{ std::move(card_hist) }, .newest_quantity_ = newest_quantity });
  }

  return CollectionHistory(std::move(card_histories), std::move(timestamps));
}

}// namespace mtgo