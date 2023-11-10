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
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace mtgo_preprocessor::run {


using cfg = config::Config;


/// Helper functions (not part of the public API)
namespace helper {

  /// Check if the state_log in the collection-history is different from the one in the appdata directory
  ///
  /// If there is no state_log in the collection-history, returns true.
  [[nodiscard]] auto has_state_log_changed(std::string_view appdata_dir_path) -> bool
  {
    const std::string state_log = "state_log.toml";
    std::filesystem::path history_log_fullpath = appdata_dir_path;
    history_log_fullpath.append("collection-history");
    history_log_fullpath.append(state_log);

    spdlog::info("Getting state_log from {}", history_log_fullpath.string());

    if (std::filesystem::exists(history_log_fullpath)) {
      std::filesystem::path appdata_log_fullpath = appdata_dir_path;
      appdata_log_fullpath.append(state_log);

      if (auto cmp_res = io_util::is_files_equal(appdata_log_fullpath, history_log_fullpath); cmp_res.has_error()) {
        spdlog::error("{}", cmp_res.error());
        return false;
      } else {
        return !cmp_res.value();
      }
    } else {
      return true;
    }
  }


  struct [[nodiscard]] JsonAndDestinationDir
  {
    std::string_view json;
    std::string_view dir;
  };

  /// Write the JSON to a file in the appdata directory
  auto write_json_to_appdata_dir(JsonAndDestinationDir jsonAndDir) -> outcome::result<void, std::string>
  {
    const std::string fname{ "mtgo-cards" };
    const std::string ext{ ".json" };
    std::filesystem::path save_path = jsonAndDir.dir;
    save_path.append("collection-history");
    save_path.append(fname);

    spdlog::info("Saving preprocessed JSON to {}", save_path.string());

    // Check if the state_log in the appdata has changed
    // If it has, save a copy of the new state_log to the collection-history directory
    //  and save the JSON to the appdata directory
    if (has_state_log_changed(jsonAndDir.dir)) {
      spdlog::info("State log has changed, saving new file");
      // Save the state_log to the collection-history directory as well
      const std::string state_log = "state_log.toml";
      std::filesystem::path appdata_log_fullpath = jsonAndDir.dir;
      appdata_log_fullpath.append(state_log);

      std::filesystem::path log_fullpath_collection_history = jsonAndDir.dir;
      log_fullpath_collection_history.append("collection-history");
      // If the collection-history directory does not exist, create it
      if (!std::filesystem::exists(log_fullpath_collection_history)) {
        std::filesystem::create_directory(log_fullpath_collection_history);
      }
      log_fullpath_collection_history.append(state_log);

      std::filesystem::copy_file(
        appdata_log_fullpath, log_fullpath_collection_history, std::filesystem::copy_options::overwrite_existing);

    } else {
      spdlog::info("State log has not changed, return without saving new file");
      return outcome::success();
    }

    if (auto res = io_util::save_with_timestamp(jsonAndDir.json, save_path, ext); res.has_error()) {
      return outcome::failure(res.error());
    } else {
      return outcome::success();
    }
  }

  /// Read the data from the scryfall JSON file into a map.
  /// Then call the extract function on the collection to get all the useful data.
  [[nodiscard]] auto parse_scryfall_data(mtgo::Collection &collection) -> outcome::result<void, std::string>
  {
    if (auto scryfall_path = cfg::get()->OptionValue(config::option::scryfall_path)) {
      if (auto scryfall_vec = scryfall::ReadJsonVector(scryfall_path.value())) {
        collection.ExtractScryfallInfo(std::move(scryfall_vec.value()));
        return outcome::success();
      } else {
        return outcome::failure(fmt::format("Expected a vector of scryfall card data: {}", scryfall_vec.error()));
      }
    } else {
      return outcome::failure("No scryfall path specified");
    }
  }


  /// Check if the next released set from the TOML state_log is now in the card definitions
  ///
  /// If it is, clear it from the state_log by replacing the values with empty strings.
  [[nodiscard]] auto update_state_log_set(std::string_view appdata_dir, const goatbots::card_defs_map_t &card_defs)
    -> outcome::result<void, std::string>
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
        return outcome::failure(fmt::format("Failed to open state_log for writing: {}", log_fullpath));
      }
    }
    return outcome::success();
  }

  /// Check if the arguments for the Goatbots card definitions and price history paths are set.
  /// If they are, check if they have values.
  ///
  /// On success: return `GoatbotsPaths` (struct with the paths)
  [[nodiscard]] auto get_goatbots_path_args() -> outcome::result<GoatbotsPaths, ErrorStr>
  {
    // First check if the arguments are set
    const bool arg_set_card_defs_path = cfg::get()->FlagSet(config::option::card_defs_path);
    const bool arg_set_price_hist_path = cfg::get()->FlagSet(config::option::price_hist_path);

    if (!arg_set_card_defs_path && !arg_set_price_hist_path) {
      return outcome::failure("Card definitions and price history path options not provided");
    }

    if (!arg_set_card_defs_path) { return outcome::failure("Card definitions path option not provided"); }

    if (!arg_set_price_hist_path) { return outcome::failure("Price history path option not provided"); }

    // Check if they have values
    auto card_defs_path = cfg::get()->OptionValue(config::option::card_defs_path);
    auto price_hist_path = cfg::get()->OptionValue(config::option::price_hist_path);

    if (!card_defs_path.has_value() && !price_hist_path.has_value()) {
      return outcome::failure("Missing card definitions and price history path from arguments");
    }

    if (!card_defs_path.has_value()) { return outcome::failure("Missing card definitions path from arguments"); }

    if (!price_hist_path.has_value()) { return outcome::failure("Missing price history path from arguments"); }

    return outcome::success(
      GoatbotsPaths{ .card_defs_path = card_defs_path.value(), .price_hist_path = price_hist_path.value() });
  }

}// namespace helper

[[nodiscard]] auto parse_goatbots_data(mtgo::Collection &mtgo_collection, GoatbotsPaths paths)
  -> outcome::result<Success, ErrorStr>
{
  // Get card definitions as a map
  auto res_card_defs = goatbots::ReadJsonMap<goatbots::card_defs_map_t>(paths.card_defs_path);
  if (res_card_defs.has_error()) { return outcome::failure(res_card_defs.error()); }

  // Get price history as a map
  auto res_price_hist = goatbots::ReadJsonMap<goatbots::price_hist_map_t>(paths.price_hist_path);
  if (res_price_hist.has_error()) { return outcome::failure(res_price_hist.error()); }

  // Extract data from the maps
  if (res_price_hist.has_value() && res_card_defs.has_value()) {
    mtgo_collection.ExtractGoatbotsInfo(res_card_defs.value(), res_price_hist.value());

    spdlog::info("GB Extraction complete, checking state_log");

    // Check if the next released set from the TOML state_log is now in the card definitions
    //
    // If it is, clear it from the state_log by replacing the values with empty strings.
    if (auto appdata_dir = cfg::get()->OptionValue(config::option::app_data_dir)) {
      if (auto res = helper::update_state_log_set(appdata_dir.value(), res_card_defs.value()); res.has_error()) {
        return outcome::failure(res.error());
      }
    }

  } else {
    return outcome::failure("Price history or card definitions missing, this should be unreachable...");
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
  auto res_mtgo_cards = mtgo::xml::parse_dek_xml(fulltradelist_path.value());
  if (res_mtgo_cards.has_error()) { return outcome::failure(res_mtgo_cards.error()); }
  auto mtgo_collection = mtgo::Collection(std::move(res_mtgo_cards.value()));


  if (auto goatbots_path_args = helper::get_goatbots_path_args(); goatbots_path_args.has_error()) {
    return outcome::failure(goatbots_path_args.error());
  } else {
    if (auto res = parse_goatbots_data(mtgo_collection, goatbots_path_args.value()); res.has_error()) {

      return outcome::failure(res.error());
    }
    spdlog::info("extract Goatbots info complete");
  }


  if (!cfg::get()->FlagSet(config::option::scryfall_path)) {
    spdlog::error("Update all needs a path to a scryfall json-data file");
  } else {
    if (auto res = helper::parse_scryfall_data(mtgo_collection); res.has_error()) {
      return outcome::failure(res.error());
    }
    spdlog::info("extract Scryfall info completed");
  }

  // Convert the collection data to JSON
  auto json = mtgo_collection.ToJson();

  // If the app data directory is set, save it there
  if (auto appdata_dir = cfg::get()->OptionValue(config::option::app_data_dir)) {
    // Write the json to a file in the appdata directory
    if (auto res =
          helper::write_json_to_appdata_dir(helper::JsonAndDestinationDir{ .json = json, .dir = appdata_dir.value() });
        res.has_error()) {
      spdlog::error("{}", res.error());// This is bad, but not fatal.
    }
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