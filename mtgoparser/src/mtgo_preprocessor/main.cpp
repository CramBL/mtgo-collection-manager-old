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

// Paths relative to ROOT=test/test-data
const auto test_path_trade_list_small_5cards = "/mtgo/Full Trade List-small-5cards.dek";
const auto test_path_goatbots_card_defs_small = "/goatbots/card-defs-small-5cards.json";
const auto test_path_goatbots_price_hist_small = "/goatbots/price-hist-small-5cards.json";
// Relative to project root
// const auto top_test_dir_path = "../test/test-data";
const auto top_path_scryfall_default_small_500cards = "../test/test-data/mtgogetter-out/scryfall-small-100cards.json";
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
  clap::Option("--verbose", true),
  clap::Option("--echo", true),
  clap::Option("--caller", false, "--calling"),
  clap::Option("--test-dir", false, "--data-dir"),
  clap::Option("--example-json", true),
  clap::Option("--example-json-formats", true),
  clap::Option("--example-scryfall", true),
  clap::Option("--gui-example", true),
  mtgoupdater_json_out);

constinit auto config = clap::init_clap(opt_array, clap::def_cmds(clap::Command("run", true)));

namespace example {
using goatbots::card_defs_map_t;
using goatbots::price_hist_map_t;


auto goatbots_card_definitions_parse(const std::string &test_data_dir) -> std::optional<card_defs_map_t>
{
  spdlog::info("=> parsing goatbots card definitions from {}...", test_data_dir + test_path_goatbots_card_defs_small);
  std::optional<card_defs_map_t> card_defs =
    goatbots::ReadJsonMap<card_defs_map_t>(test_data_dir + test_path_goatbots_card_defs_small);
  if (!card_defs.has_value()) {
    // Error: ReadJsonMap() failed
    return std::nullopt;
  }
  return card_defs;
}

auto goatbots_price_history_parse(const std::string &test_data_dir) -> price_hist_map_t
{
  spdlog::info(
    "=> parsing goatbots price history json from {}...", test_data_dir + test_path_goatbots_price_hist_small);
  price_hist_map_t prices =
    goatbots::ReadJsonMap<price_hist_map_t>(test_data_dir + test_path_goatbots_price_hist_small).value();
  return prices;
}

auto scryfall_cards_parse() -> scryfall::scryfall_card_vec
{
  spdlog::info("=> parsing scryfall json from {}...", top_path_scryfall_default_small_500cards);

  auto scryfall_card_vec = scryfall::ReadJsonVector(top_path_scryfall_default_small_500cards);

  if (config.FlagSet("--verbose")) {
    fmt::print("Deserialized scryfall cards:\n");
    for (const auto &c : scryfall_card_vec.value()) {
      fmt::print("mtgo_id: {}\n", c.mtgo_id);
      fmt::print("name: {}\n", c.name);
      fmt::print("rarity: {}\n", c.rarity);
      fmt::print("released_at: {}\n", c.released_at);
      if (c.prices.eur.has_value()) {
        fmt::print("\tprices.eur: {}\n", c.prices.eur.value());
      } else {
        fmt::print("\tprices.eur: null\n");
      }
      if (c.prices.eur_foil.has_value()) {
        fmt::print("\tprices.eur_foil: {}\n", c.prices.eur_foil.value());
      } else {
        fmt::print("\tprices.eur_foil: null\n");
      }
      if (c.prices.usd.has_value()) {
        fmt::print("\tprices.usd: {}\n", c.prices.usd.value());
      } else {
        fmt::print("\tprices.usd: null\n");
      }
      if (c.prices.usd_foil.has_value()) {
        fmt::print("\tprices.usd_foil: {}\n", c.prices.usd_foil.value());
      } else {
        fmt::print("\tprices.usd_foil: null\n");
      }
      if (c.prices.tix.has_value()) {
        fmt::print("\tprices.tix: {}\n", c.prices.tix.value());
      } else {
        fmt::print("\tprices.tix: null\n");
      }
    }
  }

  return scryfall_card_vec.value();
}

void collection_parse(const std::string &test_data_dir)
{
  spdlog::info("=== example_collection_parse ===");


  spdlog::info("==> parsing goatbots json...");
  auto card_defs = goatbots_card_definitions_parse(test_data_dir);

  price_hist_map_t prices = goatbots_price_history_parse(test_data_dir);

  spdlog::info("==> parsing mtgo xml...");
  auto cards = mtgo::xml::parse_dek_xml(test_data_dir + test_path_trade_list_small_5cards);
  auto collection = mtgo::Collection(std::move(cards));

  auto scryfall_cards = scryfall_cards_parse();
  spdlog::info("got {} scryfall cards", scryfall_cards.size());
  spdlog::warn("TODO: Parse scryfall cards into collection info");

  spdlog::info("==> collection extract goatbots info...");
  collection.ExtractGoatbotsInfo(card_defs.value(), prices);

  spdlog::info("Collection size: {}", collection.Size());
  spdlog::info("==> full collection print...");
  collection.Print();
  spdlog::info("==> collection to json...");
  spdlog::info("{}", collection.ToJson());
  spdlog::info("==> collection to pretty json...");
  spdlog::info("{}", collection.ToJsonPretty());

  spdlog::info("==> collection to json string...");
  auto collection_json = collection.ToJson();
  spdlog::info("==> override collection from json string...");
  collection.FromJson(collection_json);
  collection.Print();
  spdlog::info("==> new collection from json string...");
  auto new_collection = mtgo::Collection(collection_json);
  spdlog::info("==> new collection print...");
  new_collection.Print();
}

void collection_parse_to_gui(const std::string &test_data_dir)
{
  auto card_defs = goatbots_card_definitions_parse(test_data_dir);
  price_hist_map_t prices = goatbots_price_history_parse(test_data_dir);
  auto cards = mtgo::xml::parse_dek_xml(test_data_dir + test_path_trade_list_small_5cards);
  auto collection = mtgo::Collection(std::move(cards));
  // TODO: Add function for collection to extract scryfall into
  // auto scryfall_cards = scryfall_cards_parse();
  collection.ExtractGoatbotsInfo(card_defs.value(), prices);
  collection.PrettyPrint();
}

void json_format_prints()
{
  spdlog::info("=== example of JSON format prints for struct definitions ===");

  spdlog::info("==> Example JSON for scryfall::Card");
  spdlog::info("Printing default constructed scryfall::Card");
  auto scryfall_card = scryfall::Card{};
  std::string out_json_default_constructed_scryfall;
  glz::write_json(scryfall_card, out_json_default_constructed_scryfall);
  fmt::print("{}\n", out_json_default_constructed_scryfall);
  glz::write<glz::opts{ .prettify = true }>(scryfall_card, out_json_default_constructed_scryfall);
  fmt::print("{}\n", out_json_default_constructed_scryfall);

  spdlog::info("Default constructed but without skipping null members");
  std::string out_json_def_constr_with_null_scryfall;
  glz::write<glz::opts{ .skip_null_members = false }>(scryfall_card, out_json_def_constr_with_null_scryfall);
  fmt::print("{}\n", out_json_def_constr_with_null_scryfall);
  glz::write<glz::opts{ .skip_null_members = false, .prettify = true }>(
    scryfall_card, out_json_def_constr_with_null_scryfall);
  fmt::print("{}\n", out_json_def_constr_with_null_scryfall);

  spdlog::info("Overwriting the nested `prices` object with various values");
  scryfall_card.prices = scryfall::Prices("", std::nullopt, "20", "0.34", "");
  std::string out_json_overwritten_prices_scryfall;
  glz::write_json(scryfall_card, out_json_overwritten_prices_scryfall);
  fmt::print("{}\n", out_json_overwritten_prices_scryfall);
  glz::write<glz::opts{ .prettify = true }>(scryfall_card, out_json_overwritten_prices_scryfall);
  fmt::print("{}\n", out_json_overwritten_prices_scryfall);

  spdlog::info("Same but without skipping null members");
  std::string out_json_overwritten_prices_with_null_scryfall;
  glz::write<glz::opts{ .skip_null_members = false }>(scryfall_card, out_json_overwritten_prices_with_null_scryfall);
  fmt::print("{}\n", out_json_overwritten_prices_with_null_scryfall);
  glz::write<glz::opts{ .skip_null_members = false, .prettify = true }>(
    scryfall_card, out_json_overwritten_prices_with_null_scryfall);
  fmt::print("{}\n", out_json_overwritten_prices_with_null_scryfall);

  spdlog::info("Priting JSON schema for scryfall::Card");
  std::string schema_scryfall = glz::write_json_schema<scryfall::Card>();
  fmt::print("{}\n", schema_scryfall);

  spdlog::info("==> Example JSON for mtgo::Card");
  spdlog::info("Printing default constructed mtgo::Card");
  auto mtgo_card = mtgo::Card{};
  std::string out_json_default_constructed_mtgo;
  glz::write_json(mtgo_card, out_json_default_constructed_mtgo);
  fmt::print("{}\n", out_json_default_constructed_mtgo);
  out_json_default_constructed_mtgo.clear();
  glz::write<glz::opts{ .prettify = true }>(mtgo_card, out_json_default_constructed_mtgo);
  fmt::print("{}\n", out_json_default_constructed_mtgo);

  spdlog::info("Default constructed but without skipping null members");
  std::string out_json_def_constr_with_null_mtgo;
  glz::write_json(mtgo_card, out_json_def_constr_with_null_mtgo);
  fmt::print("{}\n", out_json_def_constr_with_null_mtgo);
  out_json_def_constr_with_null_mtgo.clear();
  glz::write<glz::opts{ .prettify = true }>(mtgo_card, out_json_def_constr_with_null_mtgo);
  fmt::print("{}\n", out_json_def_constr_with_null_mtgo);


  spdlog::info("With values");
  uint32_t id = 123;
  uint16_t quantity = 1;
  std::string_view name = "Godzilla";
  std::string_view set = "Best Set";
  std::string_view rarity = "Mythic";
  mtgo::Card mtgo_card_vals = mtgo::Card(id, quantity, name, set, rarity, true, 100.1239, 0.6);
  std::string out_json_overwritten_vals_mtgo;
  glz::write_json(mtgo_card_vals, out_json_overwritten_vals_mtgo);
  fmt::print("{}\n", out_json_overwritten_vals_mtgo);
  out_json_overwritten_vals_mtgo.clear();
  glz::write<glz::opts{ .prettify = true }>(mtgo_card_vals, out_json_overwritten_vals_mtgo);
  fmt::print("{}\n", out_json_overwritten_vals_mtgo);

  spdlog::info("Priting JSON schema for mtgo::Card");
  std::string schema_mtgo = glz::write_json_schema<mtgo::Card>();
  fmt::print("{}\n", schema_mtgo);
}

}// namespace example


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


  if (auto option_arg = config.OptionValue("--caller")) {
    spdlog::info("Called from: {}", option_arg.value());
    if (option_arg.value() == "mtgoupdater") {
      test_data_dir.assign("../test/test-data");
      spdlog::info("Setting test directory to: {}\n", test_data_dir);
    }
  } else if (auto option_test_dir_arg = config.OptionValue("--test-dir")) {
    test_data_dir.assign(option_test_dir_arg.value());
    spdlog::info("Setting test directory to: {}\n", option_test_dir_arg.value());
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


    spdlog::info("MTGO Preprocessor run mode");
    if (config.FlagSet("--example-json")) {
      example::collection_parse(test_data_dir);
      spdlog::info("Example complete!");
    }

    if (config.FlagSet("--example-json-formats")) { example::json_format_prints(); }

    if (config.FlagSet("--example-scryfall")) {
      auto scryfall_cards = example::scryfall_cards_parse();
      spdlog::info("got {} scryfall cards", scryfall_cards.size());
    }

    if (config.FlagSet("--gui-example")) { example::collection_parse_to_gui(test_data_dir); }

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