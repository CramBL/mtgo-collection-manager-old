#include "mtgo_preprocessor/run.hpp"

#include "mtgoparser/clap.hpp"
#include "mtgoparser/config.hpp"
#include "mtgoparser/goatbots.hpp"
#include "mtgoparser/mtgo.hpp"
#include "mtgoparser/scryfall.hpp"

#include <spdlog/spdlog.h>

#include <optional>


namespace mtgo_preprocessor::run {

using cfg = config::Config;


void parse_goatbots_data(mtgo::Collection &mtgo_collection, GoatbotsPaths paths)
{
  // Get card definitions as a map
  auto card_defs = goatbots::ReadJsonMap<goatbots::card_defs_map_t>(paths.card_defs_path);
  assert(card_defs.has_value());

  // Get price history as a map
  auto price_hist = goatbots::ReadJsonMap<goatbots::price_hist_map_t>(paths.price_hist_path);
  assert(price_hist.has_value());

  // Extract data from the maps
  mtgo_collection.ExtractGoatbotsInfo(card_defs.value(), price_hist.value());
  spdlog::info("extract Goatbots info complete");
}


int update()
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

    parse_goatbots_data(mtgo_collection,
      GoatbotsPaths{ .card_defs_path = card_defs_path.value(), .price_hist_path = price_hist_path.value() });

    spdlog::info("extract Goatbots info complete");
  }


  if (!cfg::get()->FlagSet(config::option::scryfall_path)) {
    spdlog::error("Update all needs a path to a scryfall json-data file");
  } else {
    auto scryfall_path = cfg::get()->OptionValue(config::option::scryfall_path);
    assert(scryfall_path.has_value());
    auto scryfall_vec = scryfall::ReadJsonVector(scryfall_path.value());
    assert(scryfall_vec.has_value());

    mtgo_collection.ExtractScryfallInfo(std::move(scryfall_vec.value()));
    spdlog::info("extract Scryfall info completed");
  }

  // Convert the collection data to JSON
  auto json = mtgo_collection.ToJson();

  // If the app data directory is set, save it there
  if (auto appdata_dir = cfg::get()->OptionValue(config::option::app_data_dir)) {
    // Write the json to a file in the appdata directory
    std::string mtgo_cards_json_fname = "mtgo-cards.json";
    std::string fullpath = std::string(appdata_dir.value()) + mtgo_cards_json_fname;
    std::ofstream mtgo_cards_outfile(fullpath);
    if (mtgo_cards_outfile.is_open()) {
      mtgo_cards_outfile << json << std::endl;
      mtgo_cards_outfile.close();
    }
  }

  // Print the MTGO collection JSON to stdout
  fmt::print("{}", json);
  return 0;
}

int run()
{
  if (cfg::get()->FlagSet(config::option::update)) { return update(); }
  return 0;
}

}// namespace mtgo_preprocessor::run