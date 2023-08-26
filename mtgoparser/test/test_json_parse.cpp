#include <catch2/catch_test_macros.hpp>

#include "mtgoparser/goatbots.hpp"
#include <utility>

const auto path_goatbots_card_defs_small_5cards = "../../test/test-data/goatbots/card-defs-small-5cards.json";
const auto path_goatbots_price_hist_small_5cards = "../../test/test-data/goatbots/price-hist-small-5cards.json";

TEST_CASE("CardDefinition structs are correctly deserialized from Goatbots JSON", "[card_defs_from_goatbots_json]")
{
  using goatbots::card_defs_map_t;
  using goatbots::CardDefinition;

  std::optional<card_defs_map_t> card_defs_opt =
    goatbots::ReadJsonMap<card_defs_map_t>(path_goatbots_card_defs_small_5cards);

  REQUIRE(card_defs_opt.has_value());
  if (!card_defs_opt.has_value()) { return; }// Make the compiler shut up
  const auto card_defs = card_defs_opt.value();

  SECTION("Sanity tests - Card definitions")
  {
    REQUIRE(card_defs.size() == 5);
    CHECK(card_defs.contains("47483"));
    CHECK(card_defs.contains("40516"));
    CHECK(card_defs.contains("31745"));
    CHECK(card_defs.contains("348"));
    CHECK(card_defs.contains("347"));

    SECTION("Card Definitions map lookup returns correct data")
    {
      CHECK(card_defs.at("47483").name == "Gruul Charm");
      CHECK(card_defs.at("47483").cardset == "GTC");
      CHECK(card_defs.at("47483").rarity == "Uncommon");
      CHECK(card_defs.at("47483").foil == 0);

      const char *const windfall_id = "40516";
      auto expect_windfall_def = CardDefinition{ "Windfall", "CMD", "Uncommon", 0 };
      auto windfall_def = card_defs.at(windfall_id);
      CHECK(windfall_def == expect_windfall_def);
    }
  }
}

TEST_CASE("Card prices are correctly deserialized from Goatbots JSON", "[prices_from_goatbots_json]")
{
  using goatbots::price_hist_map_t;
  std::optional<price_hist_map_t> prices_opt =
    goatbots::ReadJsonMap<price_hist_map_t>(path_goatbots_price_hist_small_5cards);

  REQUIRE(prices_opt.has_value());
  if (!prices_opt.has_value()) { return; }// Make the compiler shut up

  const auto prices = prices_opt.value();

  SECTION("Sanity tests - Card prices")
  {
    REQUIRE(prices.size() == 5);
    CHECK(prices.contains("112348"));
    CHECK(prices.contains("40516"));
    CHECK(prices.contains("31745"));
    CHECK(prices.contains("348"));
    CHECK(prices.contains("347"));
  }
}