#if _MSC_VER && !__INTEL_COMPILER
// On MSVC: Disable warning "discarding return value of function with 'nodiscard' attribute"
//  Because they warn on their own std::vector implementation, a warning that is discouraged by the standard...
#pragma warning(disable : 4834)
#endif

#include "mtgoparser/clap.hpp"
#include "mtgoparser/goatbots.hpp"
#include "mtgoparser/io.hpp"
#include "mtgoparser/mtgo.hpp"
#include "mtgoparser/scryfall.hpp"
#include <algorithm>
#include <cassert>
#include <internal_use_only/config.hpp>
#include <optional>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <string_view>
#include <type_traits>

// TODO: TEMPORARY NOLINT - remove when all examples are gone and it's out of early development.
// NOLINTBEGIN

// Relative to a subproject
const auto path_mtgogetter_out_scryfall_full = "../test/test-data/mtgogetter-out/scryfall-20231002-full.json";
const auto path_trade_list_medium_3000cards = "../test/test-data/mtgo/Full Trade List-medium-3000cards.dek";
const auto path_goatbots_card_defs_full = "../test/test-data/goatbots/card-definitions-2023-10-02-full.json";
const auto path_goatbots_price_hist_full = "../test/test-data/goatbots/price-history-2023-10-02-full.json";


constexpr clap::Option mtgoupdater_json_out{ "--collection-json-out", true };
constexpr clap::Option help_opt{ "-h", true };
constexpr clap::Option debug_opt{ "-d", true, "--debug" };
constexpr clap::Option update_opt{ "-u", true, "--update", "--update-all" };
constexpr clap::Option scryfall_path_opt{ "--scryfall-path", false };
constexpr clap::Option fulltradelist_path_opt{ "--full-trade-list", false };
constexpr clap::Option card_defs_path_opt{ "--card-definitions", false };
constexpr clap::Option price_hist_path_opt{ "--price-history", false };
constexpr clap::OptionArray opt_array = clap::def_options(clap::Option("--version", true, "-V"),
  help_opt,
  debug_opt,
  update_opt,
  scryfall_path_opt,
  fulltradelist_path_opt,
  card_defs_path_opt,
  price_hist_path_opt,
  clap::Option("--echo", true),
  mtgoupdater_json_out);

constinit auto config = clap::init_clap(opt_array, clap::def_cmds(clap::Command("run", true)));


int main(int argc, char *argv[])
{
  // https://github.com/gabime/spdlog/wiki/0.-FAQ#switch-the-default-logger-to-stderr
  spdlog::set_default_logger(spdlog::stderr_color_st("rename_default_logger_to_keep_format"));
  spdlog::set_default_logger(spdlog::stderr_color_st(""));

  std::string test_data_dir{ "./../test/test-data" };

  // Parse (and validate) command-line arguments
  if (auto errors = config.Parse(argc, argv)) {
    spdlog::error("{} arguments failed to validate", errors);
    return 1;
  };

  if (config.FlagSet(help_opt)) {
    config.PrintShortHelp();
    return 0;
  }

  if (config.FlagSet("--echo")) { config.PrintArgs(); }

  if (config.FlagSet("--version")) { fmt::println("v{}", mtgoparser::cmake::project_version); }

  if (config.CmdSet("run")) {

    if (config.FlagSet(update_opt)) {
      // Parse collection
      if (!config.FlagSet(scryfall_path_opt)) {
        spdlog::error("Update all needs a path to a scryfall json-data file");
        return -1;
      }
      if (!config.FlagSet(fulltradelist_path_opt)) {
        spdlog::error("Update all needs a path to a full trade list file");
        return -1;
      }
      if (!config.FlagSet(card_defs_path_opt)) {
        spdlog::error("Update all needs a path to a card definition file");
        return -1;
      }
      if (!config.FlagSet(price_hist_path_opt)) {
        spdlog::error("Update all needs a path to a price history file");
        return -1;
      }

      auto scryfall_path = config.OptionValue(scryfall_path_opt);
      assert(scryfall_path.has_value());
      auto scryfall_vec = scryfall::ReadJsonVector(scryfall_path.value());
      assert(scryfall_vec.has_value());

      auto card_defs_path = config.OptionValue(card_defs_path_opt);
      assert(card_defs_path.has_value());
      auto card_defs = goatbots::ReadJsonMap<goatbots::card_defs_map_t>(card_defs_path.value());
      assert(card_defs.has_value());

      auto price_hist_path = config.OptionValue(price_hist_path_opt);
      assert(price_hist_path.has_value());
      auto price_hist = goatbots::ReadJsonMap<goatbots::price_hist_map_t>(price_hist_path.value());
      assert(price_hist.has_value());

      auto fulltradelist_path = config.OptionValue(fulltradelist_path_opt);
      assert(fulltradelist_path.has_value());
      auto mtgo_cards = mtgo::xml::parse_dek_xml(fulltradelist_path.value());
      auto mtgo_collection = mtgo::Collection(std::move(mtgo_cards));
      mtgo_collection.ExtractGoatbotsInfo(card_defs.value(), price_hist.value());
      spdlog::info("extracted Goatbots info");
      mtgo_collection.ExtractScryfallInfo(std::move(scryfall_vec.value()));
      spdlog::info("extracted Scryfall info");
      auto json = mtgo_collection.ToJson();
      fmt::print("{}", json);
      return 0;
    }

    if (config.FlagSet(mtgoupdater_json_out.name_)) {
      auto scryfall_vec = scryfall::ReadJsonVector(path_mtgogetter_out_scryfall_full);
      spdlog::info("got scryfall vec");
      assert(scryfall_vec.has_value());
      auto card_defs = goatbots::ReadJsonMap<goatbots::card_defs_map_t>(path_goatbots_card_defs_full);
      spdlog::info("got card defs");
      assert(card_defs.has_value());
      auto price_hist = goatbots::ReadJsonMap<goatbots::price_hist_map_t>(path_goatbots_price_hist_full);
      spdlog::info("got price hist");
      assert(price_hist.has_value());
      auto mtgo_cards = mtgo::xml::parse_dek_xml(path_trade_list_medium_3000cards);
      spdlog::info("got mtgo cards");
      auto mtgo_collection = mtgo::Collection(std::move(mtgo_cards));

      mtgo_collection.ExtractGoatbotsInfo(card_defs.value(), price_hist.value());
      spdlog::info("extracted Goatbots info");
      mtgo_collection.ExtractScryfallInfo(std::move(scryfall_vec.value()));
      spdlog::info("extracted Scryfall info");

      auto pretty_json_str = mtgo_collection.ToJsonPretty();
      fmt::print("{}", pretty_json_str);
    }
  }


  return 0;
}

// NOLINTEND