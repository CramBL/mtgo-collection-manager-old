#include "mtgoparser/goatbots.hpp"
#include "mtgoparser/io.hpp"
#include "mtgoparser/mtgo.hpp"
#include <internal_use_only/config.hpp>
#include <spdlog/spdlog.h>

const auto path_trade_list_small_5cards = "./test/test-data/mtgo/Full Trade List-small-5cards.dek";
const auto path_goatbots_card_defs_small = "./test/test-data/goatbots/card-defs-small-5cards.json";
const auto path_goatbots_price_hist_small = "./test/test-data/goatbots/price-hist-small-5cards.json";

// TODO: VERY TEMPORARY NOLINT - AS SOON AS MAIN DOES SOMETHING USEFUL REMOVE THIS!!
// NOLINTBEGIN

auto example_collection_parse() -> int
{
  spdlog::info("=== example_collection_parse ===");
  using goatbots::card_defs_map_t;
  using goatbots::CardDefinition;
  using goatbots::price_hist_map_t;

  spdlog::info("==> parsing goatbots json...");
  std::optional<card_defs_map_t> card_defs = goatbots::ReadJsonMap<card_defs_map_t>(path_goatbots_card_defs_small);
  if (!card_defs.has_value()) {
    // Error: ReadJsonMap() failed
    return 1;
  }
  price_hist_map_t prices = goatbots::ReadJsonMap<price_hist_map_t>(path_goatbots_price_hist_small).value();
  spdlog::info("==> parsing mtgo xml...");
  auto cards = mtgo::xml::parse_dek_xml(path_trade_list_small_5cards);
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

int main()
{
  fmt::print("v{}\n", mtgoparser::cmake::project_version);
  return example_collection_parse();
}

// NOLINTEND