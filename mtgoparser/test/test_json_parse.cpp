#include <catch2/catch_test_macros.hpp>

#include "mtgoparser/goatbots.hpp"
#include <utility>

const auto path_goatbots_card_defs_small_5cards = "../../test/test-data/goatbots/card-defs-small-5cards.json";
const auto path_goatbots_price_hist_small_5cards = "../../test/test-data/goatbots/price-hist-small-5cards.json";

TEST_CASE("CardDefinition structs are correctly deserialized from Goatbots JSON", "[cards_from_goatbots_json]")
{

  using goatbots::card_defs_map_t;
  using goatbots::CardDefinition;
  using goatbots::price_hist_map_t;

  std::optional<card_defs_map_t> card_defs =
    goatbots::ReadJsonMap<card_defs_map_t>(path_goatbots_card_defs_small_5cards);

  SECTION("Sanity tests - Card definitions") { REQUIRE(card_defs.has_value()); }
}
