#include "collection.hpp"
#include "goatbots.hpp"
#include "io.hpp"
#include "mtgo.hpp"
#include <spdlog/spdlog.h>

void example_goatbots_json_parse()
{
  spdlog::info("=== example_goatbots_json_parse ===");
  using goatbots::card_defs_map_t;
  using goatbots::CardDefinition;
  using goatbots::price_hist_map_t;

  std::optional<card_defs_map_t> cards =
    goatbots::ReadJsonMap<card_defs_map_t>("./test/test-data/card-defs-small.json");
  if (!cards.has_value()) {
    // Error: ReadJsonMap() failed
  }
  price_hist_map_t prices = goatbots::ReadJsonMap<price_hist_map_t>("./test/test-data/price-hist-small.json").value();

  for (auto &&e : cards.value()) {
    if (e.second.name == "Black Lotus") { spdlog::info("{} : {} : {}", e.first, e.second.name, prices.at(e.first)); }
  }
}

void example_mtgo_xml_parse()
{
  spdlog::info("=== example_mtgo_xml_parse ===");
  auto cards = mtgo::xml::parse_dek_xml("./test/test-data/Full Trade List-small.dek");

  for (auto &&c : cards) {
    spdlog::info("id: {}, quantity: {}, name: {}, set: {}", c.id_, c.quantity_, c.name_, c.set_);
  }

  auto collection = mtgo::Collection(std::move(cards));
  spdlog::info("Collection size (card count): {}", collection.Size());
}

void example_collection_parse()
{
  spdlog::info("=== example_collection_parse ===");
  using goatbots::card_defs_map_t;
  using goatbots::CardDefinition;
  using goatbots::price_hist_map_t;

  spdlog::info("==> parsing goatbots json...");
  std::optional<card_defs_map_t> card_defs =
    goatbots::ReadJsonMap<card_defs_map_t>("./test/test-data/card-defs-small.json");
  if (!card_defs.has_value()) {
    // Error: ReadJsonMap() failed
  }
  price_hist_map_t prices = goatbots::ReadJsonMap<price_hist_map_t>("./test/test-data/price-hist-small.json").value();
  spdlog::info("==> parsing mtgo xml...");
  auto cards = mtgo::xml::parse_dek_xml("./test/test-data/Full Trade List-small.dek");
  auto collection = mtgo::Collection(std::move(cards));
  spdlog::info("==> collection extract goatbots info...");
  collection.ExtractGoatbotsInfo(card_defs.value(), prices);

  spdlog::info("Collection size: {}", collection.Size());
  collection.Print();
}

int main()
{

  example_goatbots_json_parse();
  example_mtgo_xml_parse();
  example_collection_parse();

  return 0;
}