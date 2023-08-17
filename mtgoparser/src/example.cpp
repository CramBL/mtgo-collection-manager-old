#include "goatbots.hpp"
#include "io.hpp"
#include "mtgo.hpp"

void example()
{
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

int main()
{

  // example();

  mtgo::parse_dek_xml("./test/test-data/Full Trade List-small.dek");

  return 0;
}