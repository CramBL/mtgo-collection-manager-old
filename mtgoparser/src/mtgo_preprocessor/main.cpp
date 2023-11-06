#if _MSC_VER && !__INTEL_COMPILER
// On MSVC: Disable warning "discarding return value of function with 'nodiscard' attribute"
//  Because they warn on their own std::vector implementation, a warning that is discouraged by the standard...
#pragma warning(disable : 4834)
#endif

#include "mtgo_preprocessor/config.hpp"
#include "mtgo_preprocessor/run.hpp"
#include "mtgo_preprocessor/setup.hpp"

#include "mtgoparser/goatbots.hpp"
#include "mtgoparser/mtgo.hpp"
#include "mtgoparser/mtgo/xml.hpp"
#include "mtgoparser/scryfall.hpp"

#include <fmt/core.h>
#include <internal_use_only/config.hpp>
#include <spdlog/spdlog.h>

#include <cassert>
#include <exception>
#include <string_view>
#include <utility>
#include <vector>


using cfg = config::Config;

// Relative to a subproject
const auto path_mtgogetter_out_scryfall_full = "../test/test-data/mtgogetter-out/scryfall-20231002-full.json";
const auto path_trade_list_medium_3000cards = "../test/test-data/mtgo/Full Trade List-medium-3000cards.dek";
const auto path_goatbots_card_defs_full = "../test/test-data/goatbots/card-definitions-2023-10-02-full.json";
const auto path_goatbots_price_hist_full = "../test/test-data/goatbots/price-history-2023-10-02-full.json";

// Run the example with `--debug --mtgoupdater-json-out`
int example()
{
  auto mtgo_cards = mtgo::xml::parse_dek_xml(path_trade_list_medium_3000cards);
  assert(mtgo_cards.has_value());
  spdlog::info("got mtgo cards");
  auto mtgo_collection = mtgo::Collection(std::move(mtgo_cards.value()));

  auto scryfall_vec = scryfall::ReadJsonVector(path_mtgogetter_out_scryfall_full);
  if (scryfall_vec) {
    spdlog::info("got scryfall vec");
  } else {
    spdlog::error("{}", scryfall_vec.error());
  }

  auto card_defs = goatbots::ReadJsonMap<goatbots::card_defs_map_t>(path_goatbots_card_defs_full);
  spdlog::info("got card defs");
  assert(card_defs.has_value());

  auto price_hist = goatbots::ReadJsonMap<goatbots::price_hist_map_t>(path_goatbots_price_hist_full);
  spdlog::info("got price hist");
  assert(price_hist.has_value());

  if (card_defs.has_value() && price_hist.has_value()) {
    mtgo_collection.ExtractGoatbotsInfo(card_defs.value(), price_hist.value());
    spdlog::info("extracted Goatbots info");
  } else {
    return -1;
  }

  if (scryfall_vec) {
    mtgo_collection.ExtractScryfallInfo(std::move(scryfall_vec.value()));
    spdlog::info("extracted Scryfall info");
  } else {
    spdlog::error("Extraction failed due to missing scryfall info");
    return -1;
  }

  auto pretty_json_str = mtgo_collection.ToJsonPretty();
  fmt::print("{}", pretty_json_str);
  return 0;
}

int main(int argc, char *argv[])
{
  try {

    std::vector<std::string_view> args{ argv + 1, argv + argc };

    if (auto res = mtgo_preprocessor::setup::setup(args); res.has_error()) {
      spdlog::error("{}", res.error());
      return -1;
    }


    if (cfg::get()->FlagSet(config::option::help)) {
      cfg::get()->PrintShortHelp();
      return 0;
    }

    if (cfg::get()->FlagSet(config::option::echo)) { cfg::get()->PrintArgs(); }

    if (cfg::get()->FlagSet("--version")) {
      fmt::println("v{}", mtgoparser::cmake::project_version);
      return 0;
    }

    if (cfg::get()->CmdSet(config::commands::run)) {
      if (auto res = mtgo_preprocessor::run::run(); res.has_error()) {
        spdlog::error("{}", res.error());
        return -1;
      }
    }

    if (cfg::get()->FlagSet(config::option::debug) && cfg::get()->FlagSet(config::option::mtgoupdater_json_out)) {
      return example();
    }
  } catch (...) {
    const std::exception_ptr eptr = std::current_exception();

    try {
      if (eptr) { std::rethrow_exception(eptr); }
    } catch (const std::exception &e) {
      spdlog::error("{}", e.what());
    }
  }

  return 0;
}
