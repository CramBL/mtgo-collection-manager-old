// NOLINTBEGIN
#if _MSC_VER && !__INTEL_COMPILER
// On MSVC: Disable warning C4702: unreachable code
//  This warning is generated for glaze@1.4.3 glaze\json\read.hpp l. 1574, 1577, 1580 & 1584
#pragma warning(disable : 4702)
#endif

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
using Catch::Matchers::ContainsSubstring;

#include "mtgoparser/goatbots.hpp"
#include "mtgoparser/scryfall.hpp"
#include <cstdint>
#include <optional>
#include <string>
#include <utility>

// Goes to top of project and into the shared 'test/test-data' directory
const auto path_goatbots_card_defs_small_5cards = "../../../test/test-data/goatbots/card-defs-small-5cards.json";
const auto path_goatbots_price_hist_small_5cards = "../../../test/test-data/goatbots/price-hist-small-5cards.json";
const auto path_goatbots_card_defs_full = "../../../test/test-data/goatbots/card-definitions-2023-10-02-full.json";

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
    CHECK(card_defs.contains(47483));
    CHECK(card_defs.contains(40516));
    CHECK(card_defs.contains(31745));
    CHECK(card_defs.contains(348));
    CHECK(card_defs.contains(347));

    SECTION("Card Definitions map lookup returns correct data")
    {
      CHECK(card_defs.at(47483).name == "Gruul Charm");
      CHECK(card_defs.at(47483).cardset == "GTC");
      CHECK(card_defs.at(47483).rarity == "Uncommon");
      CHECK(card_defs.at(47483).foil == 0);

      uint32_t windfall_id = 40516;
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
    CHECK(prices.contains(112348));
    CHECK(prices.contains(40516));
    CHECK(prices.contains(31745));
    CHECK(prices.contains(348));
    CHECK(prices.contains(347));
  }
}

TEST_CASE("Scryfall JSON serializing", "[scryfall_serializing]")
{
  SECTION("Default scryfall::Prices nested object (of scryfall::Card)")
  {
    scryfall::Prices prices_default{};
    REQUIRE(prices_default.usd == std::nullopt);
    REQUIRE(prices_default.usd_foil == std::nullopt);
    REQUIRE(prices_default.eur == std::nullopt);
    REQUIRE(prices_default.eur_foil == std::nullopt);
    REQUIRE(prices_default.tix == std::nullopt);

    std::string json_str_prices_default{};
    glz::write_json(prices_default, json_str_prices_default);

    REQUIRE(json_str_prices_default == R"({})");// Empty JSON object as all members are null

    SECTION("Default scryfall::Prices JSON string checks")
    {
      json_str_prices_default.clear();
      glz::write<glz::opts{ .skip_null_members = false }>(prices_default, json_str_prices_default);

      CHECK_THAT(json_str_prices_default, ContainsSubstring(R"("usd":null)"));
      CHECK_THAT(json_str_prices_default, ContainsSubstring(R"("usd_foil":null)"));
      CHECK_THAT(json_str_prices_default, ContainsSubstring(R"("eur":null)"));
      CHECK_THAT(json_str_prices_default, ContainsSubstring(R"("eur_foil":null)"));
      CHECK_THAT(json_str_prices_default, ContainsSubstring(R"("tix":null)"));
    }
  }

  SECTION("Default scryfall::Card object")
  {
    scryfall::Card card_default{};

    REQUIRE(card_default.mtgo_id == 0);
    REQUIRE(card_default.name == "");
    REQUIRE(card_default.rarity == "");
    REQUIRE(card_default.released_at == "");
    REQUIRE(card_default.prices == scryfall::Prices{});// Equal to default object


    SECTION("Default scryfall::Card object JSON string checks")
    {
      std::string json_str_card_default;
      glz::write_json(card_default, json_str_card_default);

      CHECK_THAT(json_str_card_default, ContainsSubstring(R"("mtgo_id":0)"));
      CHECK_THAT(json_str_card_default, ContainsSubstring(R"("name":"")"));
      CHECK_THAT(json_str_card_default, ContainsSubstring(R"("rarity":"")"));
      CHECK_THAT(json_str_card_default, ContainsSubstring(R"("prices":{})"));
    }

    // Now with values (not default)
    SECTION("With values scryfall::Card checks")
    {
      {
        scryfall::Card card_vals{
          0, "Mother of Runes", "2006-10-06", "rare", scryfall::Prices{ "0", "0.2", "3.0", std::nullopt, "0.05" }
        };

        // Construction with initializer list should be equivelant as
        REQUIRE(
          card_vals
          == scryfall::Card(
            0, "Mother of Runes", "2006-10-06", "rare", scryfall::Prices{ "0", "0.2", "3.0", std::nullopt, "0.05" }));
        REQUIRE(card_vals.mtgo_id == 0);
        REQUIRE(card_vals.name == "Mother of Runes");
        REQUIRE(card_vals.rarity == "rare");
        REQUIRE(card_vals.released_at == "2006-10-06");
        REQUIRE(card_vals.prices == scryfall::Prices{ "0", "0.2", "3.0", std::nullopt, "0.05" });


        SECTION("With values scryfall::Card JSON string checks")
        {
          std::string json_str_card_vals;

          glz::write_json(card_vals, json_str_card_vals);

          CHECK_THAT(json_str_card_vals, ContainsSubstring(R"("mtgo_id":0)"));
          CHECK_THAT(json_str_card_vals, ContainsSubstring(R"("name":"Mother of Runes")"));
          CHECK_THAT(json_str_card_vals, ContainsSubstring(R"("rarity":"rare")"));
          CHECK_THAT(json_str_card_vals, ContainsSubstring(R"("released_at":"2006-10-06")"));
          // Should not match this as null values are skipped by defaul (eur_foil is null)
          CHECK_THAT(json_str_card_vals,
            !ContainsSubstring(R"("prices":{"usd":"0","usd_foil":"0.2","eur":"3.0","eur_foil":null,"tix":"0.05"})"));
          CHECK_THAT(json_str_card_vals,
            !ContainsSubstring(R"("eur_foil":null)"));// Should not contain it
          // Contains this (leaving out nulled value `eur_foil`)
          CHECK_THAT(
            json_str_card_vals, ContainsSubstring(R"("prices":{"usd":"0","usd_foil":"0.2","eur":"3.0","tix":"0.05"})"));

          SECTION("With values scryfall::Card JSON string checks - not skipping null")
          {
            std::string json_str_card_vals_with_null;
            glz::write<glz::opts{ .skip_null_members = false }>(card_vals, json_str_card_vals_with_null);

            CHECK_THAT(json_str_card_vals_with_null, ContainsSubstring(R"("mtgo_id":0)"));
            CHECK_THAT(json_str_card_vals_with_null, ContainsSubstring(R"("name":"Mother of Runes")"));
            CHECK_THAT(json_str_card_vals_with_null, ContainsSubstring(R"("rarity":"rare")"));
            CHECK_THAT(json_str_card_vals_with_null, ContainsSubstring(R"("released_at":"2006-10-06")"));
            // Should now match as null values are no longer skipped
            CHECK_THAT(json_str_card_vals_with_null,
              ContainsSubstring(R"("prices":{"usd":"0","usd_foil":"0.2","eur":"3.0","eur_foil":null,"tix":"0.05"})"));
          }
        }
      }
    }
  }
}

TEST_CASE("Full JSON - CardDefinition structs deserialized from Goatbots JSON and mtgo set ID search",
  "[card_defs_full_goatbots_json]")
{
  using goatbots::card_defs_map_t;
  using goatbots::CardDefinition;

  std::optional<card_defs_map_t> card_defs_opt = goatbots::ReadJsonMap<card_defs_map_t>(path_goatbots_card_defs_full);

  REQUIRE(card_defs_opt.has_value());
  // if (!card_defs_opt.has_value()) { return; }// Make the compiler shut up
  const card_defs_map_t card_defs = card_defs_opt.value();

  SECTION("Sanity checks - card definitions")
  {
    CHECK(card_defs.size() == 76070);

    REQUIRE(card_defs.contains(116836));

    CHECK(card_defs.at(116836).name == "Pollen-Shield Hare");
    CHECK(card_defs.at(116836).foil == 0);
  }
}

// NOLINTEND