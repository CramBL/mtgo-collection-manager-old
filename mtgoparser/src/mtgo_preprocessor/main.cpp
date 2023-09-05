#if _MSC_VER && !__INTEL_COMPILER
// On MSVC: Disable warning "discarding return value of function with 'nodiscard' attribute"
//  Because they warn on their own std::vector implementation, a warning that is discouraged by the standard...
#pragma warning(disable : 4834)
#endif

#include "mtgoparser/goatbots.hpp"
#include "mtgoparser/io.hpp"
#include "mtgoparser/mtgo.hpp"
#include "mtgoparser/scryfall.hpp"
#include <algorithm>
#include <internal_use_only/config.hpp>
#include <optional>
#include <spdlog/spdlog.h>
#include <string_view>
#include <type_traits>

// TODO: VERY TEMPORARY NOLINT - AS SOON AS MAIN DOES SOMETHING USEFUL REMOVE THIS!!
// NOLINTBEGIN

const auto test_path_trade_list_small_5cards = "/mtgo/Full Trade List-small-5cards.dek";
const auto test_path_goatbots_card_defs_small = "/goatbots/card-defs-small-5cards.json";
const auto test_path_goatbots_price_hist_small = "/goatbots/price-hist-small-5cards.json";

namespace example {
auto collection_parse(const std::string &test_data_dir) -> int
{
  spdlog::info("=== example_collection_parse ===");
  using goatbots::card_defs_map_t;
  using goatbots::CardDefinition;
  using goatbots::price_hist_map_t;

  spdlog::info("==> parsing goatbots json...");
  spdlog::info("=> parsing goatbots card definitions from {}...", test_data_dir + test_path_goatbots_card_defs_small);
  std::optional<card_defs_map_t> card_defs =
    goatbots::ReadJsonMap<card_defs_map_t>(test_data_dir + test_path_goatbots_card_defs_small);
  if (!card_defs.has_value()) {
    // Error: ReadJsonMap() failed
    return 1;
  }

  spdlog::info(
    "=> parsing goatbots price history json from {}...", test_data_dir + test_path_goatbots_price_hist_small);
  price_hist_map_t prices =
    goatbots::ReadJsonMap<price_hist_map_t>(test_data_dir + test_path_goatbots_price_hist_small).value();
  spdlog::info("==> parsing mtgo xml...");
  auto cards = mtgo::xml::parse_dek_xml(test_data_dir + test_path_trade_list_small_5cards);
  auto collection = mtgo::Collection(std::move(cards));
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

  return 0;
}

void json_format_prints()
{
  spdlog::info("=== example of JSON format prints for struct definitions ===");

  spdlog::info("==> Example JSON for scryfall::Card");
  spdlog::info("Printing default constructed scryfall::Card");
  auto scryfall_card = scryfall::Card{};
  std::string out_json_default_constructed_scryfall;
  glz::write<glz::opts{ .prettify = true }>(scryfall_card, out_json_default_constructed_scryfall);
  fmt::print("{}\n", out_json_default_constructed_scryfall);

  spdlog::info("Default constructed but without skipping null members");
  std::string out_json_def_constr_with_null_scryfall;
  glz::write<glz::opts{ .skip_null_members = false, .prettify = true }>(
    scryfall_card, out_json_def_constr_with_null_scryfall);
  fmt::print("{}\n", out_json_def_constr_with_null_scryfall);

  spdlog::info("Overwriting the nested `prices` object with various values");
  scryfall_card.prices = scryfall::Prices("", std::nullopt, "20", "0.34", "");
  std::string out_json_overwritten_prices_scryfall;
  glz::write<glz::opts{ .prettify = true }>(scryfall_card, out_json_overwritten_prices_scryfall);
  fmt::print("{}\n", out_json_overwritten_prices_scryfall);

  spdlog::info("Same but without skipping null members");
  std::string out_json_overwritten_prices_with_null_scryfall;
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
  glz::write<glz::opts{ .prettify = true }>(mtgo_card, out_json_default_constructed_mtgo);
  fmt::print("{}\n", out_json_default_constructed_mtgo);

  spdlog::info("Default constructed but without skipping null members");
  std::string out_json_def_constr_with_null_mtgo;
  glz::write<glz::opts{ .skip_null_members = false, .prettify = true }>(mtgo_card, out_json_def_constr_with_null_mtgo);
  fmt::print("{}\n", out_json_def_constr_with_null_mtgo);

  spdlog::info("With values");
  std::string_view id = "123";
  std::string_view quantity = "1";
  std::string_view name = "Godzilla";
  std::string_view set = "Best Set";
  std::string_view rarity = "Mythic";
  mtgo::Card mtgo_card_vals = mtgo::Card(id, quantity, name, set, rarity, true, 100.);
  std::string out_json_overwritten_vals_mtgo;
  glz::write<glz::opts{ .skip_null_members = false, .prettify = true }>(mtgo_card_vals, out_json_overwritten_vals_mtgo);
  fmt::print("{}\n", out_json_overwritten_vals_mtgo);

  spdlog::info("Priting JSON schema for mtgo::Card");
  std::string schema_mtgo = glz::write_json_schema<mtgo::Card>();
  fmt::print("{}\n", schema_mtgo);
}

}// namespace example

// Command-Line Argument Parsing (CLAP) utility
namespace clap {
// Helper function to check if a value equals any element in a parameter pack
template<typename T, typename... Args> constexpr bool equals_any(const T &value, Args... args)
{
  return ((value == args) || ...);
}

// Type trait to check if a type is convertible to std::string_view
template<typename T> struct is_convertible_to_string_view
{
  static constexpr bool value = std::is_convertible_v<T, std::string_view>;
};

// Helper function to check if all types in a parameter pack are convertible to std::string_view
template<typename... Args> constexpr bool all_convertible_to_string_view()
{
  return (is_convertible_to_string_view<Args>::value && ...);
}

// Check if an option or any of its aliases are set
template<typename... Options>
[[nodiscard]] auto has_option(const std::vector<std::string_view> &args, Options... option_names) -> bool
{
  static_assert(all_convertible_to_string_view<Options...>(), "Options must be convertible to std::string_view");

  // Cannot use std::ranges because apple clang still does not support it...
  return std::any_of(
    args.begin(), args.end(), [&](const std::string_view &arg) { return equals_any(arg, option_names...); });
}

// Returns the argument to an option if the option or any of its aliases exists and it has an argument
template<typename... Options>
[[nodiscard]] auto has_option_arg(const std::vector<std::string_view> &args, Options... option_names)
  -> std::optional<std::string_view>
{
  static_assert(all_convertible_to_string_view<Options...>(), "Options must be convertible to std::string_view");


  for (auto it = args.begin(), end = args.end(); it != end; ++it) {
    if (equals_any(*it, option_names...)) {
      if (it + 1 != end) {
        return *(it + 1);
      } else {
        spdlog::error("Option {} was specified but no argument was given", *it);
      }
    }
  }

  return std::nullopt;
}

// Basic DIY command-line argument parsing
[[nodiscard]] std::string_view get_option(const std::vector<std::string_view> &args,
  const std::string_view &option_name)
{
  for (auto it = args.begin(), end = args.end(); it != end; ++it) {
    if (*it == option_name)
      if (it + 1 != end) return *(it + 1);
  }

  return "";
}
}// namespace clap

int main(int argc, char *argv[])
{
  std::string test_data_dir{ "./test/test-data" };

  // Get command-line arguments as a vector of string_views
  const std::vector<std::string_view> args(argv + 1, argv + argc);

  if (auto option_arg = clap::has_option_arg(args, "--caller", "--calling")) {
    fmt::print("Called from: {}\n", option_arg.value());
    if (option_arg.value() == "mtgoupdater") {
      test_data_dir.assign("../mtgoparser/test/test-data");
      fmt::print("Setting test directory to: {}\n", test_data_dir);
    }
  } else if (auto option_test_dir_arg = clap::has_option_arg(args, "--test-dir", "--data-dir")) {
    test_data_dir.assign(option_test_dir_arg.value());
    fmt::print("Setting test directory to: {}\n", option_test_dir_arg.value());
  }

  if (clap::has_option(args, "--echo")) {
    for (const auto &arg : args) { fmt::print("{}\n", arg); }
  }

  if (clap::has_option(args, "--version", "-V")) { fmt::print("v{}\n", mtgoparser::cmake::project_version); }


  if (clap::has_option(args, "--example", "--run-example", "--run")) {
    auto res = example::collection_parse(test_data_dir);
    if (res == 0) { fmt::print("Example complete!"); }
  }

  if (clap::has_option(args, "--example-json-formats", "--example-json", "--run-example-json")) {
    example::json_format_prints();
  }

  return 0;
}

// NOLINTEND