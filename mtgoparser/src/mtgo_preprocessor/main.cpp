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
#include <internal_use_only/config.hpp>
#include <optional>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <string_view>
#include <type_traits>

// TODO: VERY TEMPORARY NOLINT - AS SOON AS MAIN DOES SOMETHING USEFUL REMOVE THIS!!
// NOLINTBEGIN

const auto test_path_trade_list_small_5cards = "/mtgo/Full Trade List-small-5cards.dek";
const auto test_path_goatbots_card_defs_small = "/goatbots/card-defs-small-5cards.json";
const auto test_path_goatbots_price_hist_small = "/goatbots/price-hist-small-5cards.json";
// Relative to a project subfolder such as mtgoparser/mtgogetter/mtgoupdater
// const auto top_test_dir_path = "../test/test-data";
const auto top_path_scryfall_default_small_500cards = "../test/test-data/mtgogetter-out/scryfall-small-100cards.json";


#define OPTION_COUNT 9

constexpr clap::Option help_opt{ "-h", true };
constexpr clap::OptionArray opt_array = clap::OptionArray<OPTION_COUNT>{
  clap::Option("--version", true, "-V"),
  help_opt,
  clap::Option("--verbose", true),
  clap::Option("--echo", true),
  clap::Option("--caller", false, "--calling"),
  clap::Option("--test-dir", false, "--data-dir"),
  clap::Option("--example-json", true),
  clap::Option("--example-json-formats", true),
  clap::Option("--example-scryfall", true),
};
#define COMMAND_COUNT 1
constexpr clap::CommandArray cmd_array = clap::CommandArray<COMMAND_COUNT>{ clap::Command("run", true) };

constinit auto config = clap::Clap{ opt_array, cmd_array };

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
  std::string_view id = "123";
  std::string_view quantity = "1";
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

  std::string test_data_dir{ "./test/test-data" };

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
      test_data_dir.assign("../mtgoparser/test/test-data");
      spdlog::info("Setting test directory to: {}\n", test_data_dir);
    }
  } else if (auto option_test_dir_arg = config.OptionValue("--test-dir")) {
    test_data_dir.assign(option_test_dir_arg.value());
    spdlog::info("Setting test directory to: {}\n", option_test_dir_arg.value());
  }

  if (config.FlagSet("--echo")) { config.PrintArgs(); }

  if (config.FlagSet("--version")) { fmt::println("v{}", mtgoparser::cmake::project_version); }

  if (config.CmdSet("run")) {
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
  }


  return 0;
}

// NOLINTEND