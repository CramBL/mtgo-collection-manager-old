#include "mtgo_preprocessor/run.hpp"
#include "mtgo_preprocessor/config.hpp"

#include "mtgoparser/clap.hpp"
#include "mtgoparser/goatbots.hpp"
#include "mtgoparser/mtgo.hpp"
#include "mtgoparser/scryfall.hpp"

#include <cassert>
#include <spdlog/spdlog.h>

#include <optional>


namespace mtgo_preprocessor::run {

using cfg = config::Config;


[[nodiscard]] int parse_goatbots_data(mtgo::Collection &mtgo_collection, GoatbotsPaths paths)
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

    // Check if the next released set from the TOML state_log is now in the card definitions
    //
    // If it is, clear it from the state_log by replacing the values with empty strings.
    if (auto appdata_dir = cfg::get()->OptionValue(config::option::app_data_dir)) {
      const std::string state_log = "state_log.toml";
      const std::string log_fullpath = std::string(appdata_dir.value()) + state_log;
      decltype(auto) log = io_util::read_state_log(log_fullpath);
      std::string_view next_released_set_mtgo_code =
        log["scryfall"]["Next_released_mtgo_set"]["Mtgo_code"].value_or("");
      if (goatbots::set_id_in_card_defs(next_released_set_mtgo_code, card_defs.value())) {
        // clear the set from the statelog
        toml::value<std::string> *name = log["scryfall"]["Next_released_mtgo_set"]["Name"].as_string();
        *name = "";
        toml::value<std::string> *released_at = log["scryfall"]["Next_released_mtgo_set"]["Released_at"].as_string();
        *released_at = "";
        toml::value<std::string> *mtgo_code = log["scryfall"]["Next_released_mtgo_set"]["Mtgo_code"].as_string();
        *mtgo_code = "";
        std::ofstream replace_state_log(log_fullpath);
        if (replace_state_log.is_open()) {
          replace_state_log << log << std::endl;
          replace_state_log.close();
        } else {
          spdlog::error("Could not open state_log for writing at: {}", log_fullpath);
        }
      }
    }


  } else {
    return -1;
  }
  return 0;
}

struct JsonAndDestinationDir
{
  std::string_view json;
  std::string_view dir;
};
void write_json_to_appdata_dir(JsonAndDestinationDir jsonAndDir)
{
  const std::string mtgo_cards_json_fname = "mtgo-cards.json";
  const std::string fullpath = std::string(jsonAndDir.dir) + mtgo_cards_json_fname;
  std::ofstream mtgo_cards_outfile(fullpath);
  if (mtgo_cards_outfile.is_open()) {
    mtgo_cards_outfile << jsonAndDir.json << std::endl;
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
      spdlog::error("Expected a vector of scryfall card data");
      return -1;
    }
  } else {
    spdlog::error("Expected a path to scryfall data");
    return -1;
  }
}


[[nodiscard]] int update()
{
  // Parse collection

  // Not possible if no full trade list path is supplied
  if (!cfg::get()->FlagSet(config::option::fulltradelist_path)) {
    spdlog::error("Update all needs a path to a full trade list file");
    return -1;
  }

  // Get cards from full trade list XML
  auto fulltradelist_path = cfg::get()->OptionValue(config::option::fulltradelist_path);
  assert(fulltradelist_path.has_value());
  if (!fulltradelist_path.has_value()) { return -1; }
  auto mtgo_cards = mtgo::xml::parse_dek_xml(fulltradelist_path.value());
  auto mtgo_collection = mtgo::Collection(std::move(mtgo_cards));

  if (!cfg::get()->FlagSet(config::option::card_defs_path) || !cfg::get()->FlagSet(config::option::price_hist_path)) {
    if (!cfg::get()->FlagSet(config::option::price_hist_path)) {
      spdlog::error("Update all needs a path to a price history file");
    }
    if (!cfg::get()->FlagSet(config::option::card_defs_path)) {
      spdlog::error("Update all needs a path to a card definition file");
    }
  } else {

    // Get card definitions as a map
    auto card_defs_path = cfg::get()->OptionValue(config::option::card_defs_path);
    assert(card_defs_path.has_value());
    // Get price history as a map
    auto price_hist_path = cfg::get()->OptionValue(config::option::price_hist_path);
    assert(price_hist_path.has_value());

    if (!(card_defs_path.has_value() && price_hist_path.has_value())) { return -1; }

    if (parse_goatbots_data(mtgo_collection,
          GoatbotsPaths{ .card_defs_path = card_defs_path.value(), .price_hist_path = price_hist_path.value() })
        != 0) {
      return -1;
    }
    spdlog::info("extract Goatbots info complete");
  }


  if (!cfg::get()->FlagSet(config::option::scryfall_path)) {
    spdlog::error("Update all needs a path to a scryfall json-data file");
  } else {
    if (parse_scryfall_data(mtgo_collection) != 0) {
      spdlog::error("Error parsing scryfall data");
      return -1;
    }
    spdlog::info("extract Scryfall info completed");
  }

  // Convert the collection data to JSON
  auto json = mtgo_collection.ToJson();

  // If the app data directory is set, save it there
  if (auto appdata_dir = cfg::get()->OptionValue(config::option::app_data_dir)) {
    // Write the json to a file in the appdata directory
    write_json_to_appdata_dir(JsonAndDestinationDir{ .json = json, .dir = appdata_dir.value() });
  }

  // Print the MTGO collection JSON to stdout
  fmt::print("{}", json);
  return 0;
}

[[nodiscard]] int run()
{
  if (cfg::get()->FlagSet(config::option::update)) { return update(); }
  return 0;
}

}// namespace mtgo_preprocessor::run