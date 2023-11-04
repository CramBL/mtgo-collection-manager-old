#include "mtgo_preprocessor/run.hpp"
#include "mtgo_preprocessor/config.hpp"

#include "mtgoparser/goatbots.hpp"
#include "mtgoparser/io.hpp"
#include "mtgoparser/mtgo.hpp"
#include "mtgoparser/mtgo/xml.hpp"
#include "mtgoparser/scryfall.hpp"

#include <boost/outcome/result.hpp>
#include <boost/outcome/success_failure.hpp>
#include <fmt/core.h>
#include <spdlog/spdlog.h>
#include <toml++/impl/value.hpp>

#include <cassert>
#include <fstream>
#include <optional>
#include <string>
#include <string_view>
#include <utility>


namespace mtgo_preprocessor::run {


using cfg = config::Config;


/// Helper functions (not part of the public API)
namespace helper {
  struct JsonAndDestinationDir
  {
    std::string_view json;
    std::string_view dir;
  };

  /// Write the JSON to a file in the appdata directory
  void write_json_to_appdata_dir(JsonAndDestinationDir jsonAndDir)
  {
    const std::string mtgo_cards_json_fname = "mtgo-cards.json";
    const std::string fullpath = std::string(jsonAndDir.dir) + mtgo_cards_json_fname;
    std::ofstream mtgo_cards_outfile(fullpath);
    if (mtgo_cards_outfile.is_open()) {
      mtgo_cards_outfile << jsonAndDir.json << '\n';
      mtgo_cards_outfile.close();
    }
  }

  /// Read the data from the scryfall JSON file into a map.
  /// Then call the extract function on the collection to get all the useful data.
  [[nodiscard]] int parse_scryfall_data(mtgo::Collection &collection)
  {
    if (auto scryfall_path = cfg::get()->OptionValue(config::option::scryfall_path)) {
      if (auto scryfall_vec = scryfall::ReadJsonVector(scryfall_path.value())) {
        collection.ExtractScryfallInfo(std::move(scryfall_vec.value()));
        return 0;
      } else {
        spdlog::error("Expected a vector of scryfall card data: {}", scryfall_vec.error());
        return -1;
      }
    } else {
      spdlog::error("Expected a path to scryfall data");
      return -1;
    }
  }


  /// Check if the next released set from the TOML state_log is now in the card definitions
  ///
  /// If it is, clear it from the state_log by replacing the values with empty strings.
  void update_state_log_set(std::string_view appdata_dir, const goatbots::card_defs_map_t &card_defs)
  {
    const std::string state_log = "state_log.toml";
    const std::string log_fullpath = std::string(appdata_dir) + state_log;

    spdlog::info("Getting state_log from {}", log_fullpath);

    decltype(auto) log = io_util::read_state_log(log_fullpath);

    const std::string_view next_released_set_mtgo_code =
      log["scryfall"]["Next_released_mtgo_set"]["Mtgo_code"].value_or("");
    if (goatbots::set_id_in_card_defs(next_released_set_mtgo_code, card_defs)) {
      // clear the set from the statelog
      toml::value<std::string> *name = log["scryfall"]["next_released_mtgo_set"]["name"].as_string();
      *name = std::string("");
      toml::value<std::string> *released_at = log["scryfall"]["next_released_mtgo_set"]["released_at"].as_string();
      *released_at = std::string("");
      toml::value<std::string> *mtgo_code = log["scryfall"]["next_released_mtgo_set"]["mtgo_code"].as_string();
      *mtgo_code = std::string("");

      std::ofstream replace_state_log(log_fullpath);
      if (replace_state_log.is_open()) {
        replace_state_log << log << '\n';
        replace_state_log.close();
      } else {
        spdlog::error("Could not open state_log for writing at: {}", log_fullpath);
      }
    }
  }

}// namespace helper

[[nodiscard]] auto parse_goatbots_data(mtgo::Collection &mtgo_collection, GoatbotsPaths paths)
  -> outcome::result<Success, ErrorStr>
{
  // Get card definitions as a map
  auto card_defs = goatbots::ReadJsonMap<goatbots::card_defs_map_t>(paths.card_defs_path);
  assert(card_defs.has_value());

  // Get price history as a map
  auto price_hist = goatbots::ReadJsonMap<goatbots::price_hist_map_t>(paths.price_hist_path);
  assert(price_hist.has_value());

  // Extract data from the maps
  if (price_hist.has_value() && card_defs.has_value()) {
    mtgo_collection.ExtractGoatbotsInfo(card_defs.value(), price_hist.value());

    spdlog::info("GB Extraction complete");

    spdlog::info("checking state_log");
    // Check if the next released set from the TOML state_log is now in the card definitions
    //
    // If it is, clear it from the state_log by replacing the values with empty strings.
    if (auto appdata_dir = cfg::get()->OptionValue(config::option::app_data_dir)) {
      helper::update_state_log_set(appdata_dir.value(), card_defs.value());
    }

  } else {
    return fmt::format("This error should be unreachable...");
  }
  return outcome::success();
}


[[nodiscard]] auto update() -> outcome::result<Success, ErrorStr>
{
  // Parse collection

  // Not possible if no full trade list path is supplied
  if (!cfg::get()->FlagSet(config::option::fulltradelist_path)) {
    return outcome::failure("Update all needs a path to a full trade list XML file");
  }

  // Get cards from full trade list XML
  auto fulltradelist_path = cfg::get()->OptionValue(config::option::fulltradelist_path);
  assert(fulltradelist_path.has_value());
  if (!fulltradelist_path.has_value()) {
    return outcome::failure("Full Trade List path has no value. This error should be unreachable...");
  }
  auto mtgo_cards = mtgo::xml::parse_dek_xml(fulltradelist_path.value());
  auto mtgo_collection = mtgo::Collection(std::move(mtgo_cards));

  if (!cfg::get()->FlagSet(config::option::card_defs_path) || !cfg::get()->FlagSet(config::option::price_hist_path)) {
    if (!cfg::get()->FlagSet(config::option::price_hist_path)) {
      spdlog::error("Update all needs a path to a price history file");
    }
    if (!cfg::get()->FlagSet(config::option::card_defs_path)) {
      spdlog::error("Update all needs a path to a card definition file");
    }
    return outcome::failure("Update all needs a path to a card definition and price history file");
  } else {

    // Get card definitions as a map
    auto card_defs_path = cfg::get()->OptionValue(config::option::card_defs_path);
    assert(card_defs_path.has_value());
    // Get price history as a map
    auto price_hist_path = cfg::get()->OptionValue(config::option::price_hist_path);
    assert(price_hist_path.has_value());

    if (!(card_defs_path.has_value() && price_hist_path.has_value())) {
      return outcome::failure(
        "Failed retrieving price history and card definition path from config. This error should be unreachable...");
    }

    if (auto res = parse_goatbots_data(mtgo_collection,
          GoatbotsPaths{ .card_defs_path = card_defs_path.value(), .price_hist_path = price_hist_path.value() });
        res.has_error()) {

      return outcome::failure(res.error());
    }
    spdlog::info("extract Goatbots info complete");
  }


  if (!cfg::get()->FlagSet(config::option::scryfall_path)) {
    spdlog::error("Update all needs a path to a scryfall json-data file");
  } else {
    if (helper::parse_scryfall_data(mtgo_collection) != 0) { return outcome::failure("Failed parsing scryfall data"); }
    spdlog::info("extract Scryfall info completed");
  }

  // Convert the collection data to JSON
  auto json = mtgo_collection.ToJson();

  // If the app data directory is set, save it there
  if (auto appdata_dir = cfg::get()->OptionValue(config::option::app_data_dir)) {
    // Write the json to a file in the appdata directory
    helper::write_json_to_appdata_dir(helper::JsonAndDestinationDir{ .json = json, .dir = appdata_dir.value() });
  }

  // Print the MTGO collection JSON to stdout
  fmt::print("{}", json);
  return outcome::success();
}

[[nodiscard]] auto run() -> outcome::result<Success, ErrorStr>
{
  if (cfg::get()->FlagSet(config::option::update)) { return update(); }
  return outcome::success();
}

}// namespace mtgo_preprocessor::run