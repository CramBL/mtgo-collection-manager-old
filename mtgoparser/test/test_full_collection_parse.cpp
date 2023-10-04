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
#include "mtgoparser/mtgo.hpp"
#include "mtgoparser/scryfall.hpp"
#include <optional>
#include <string>
#include <utility>

// Used by the first section (small collection)
// Goes to top of project and into the shared 'test/test-data' directory
const auto path_goatbots_card_defs_small_5cards = "../../../test/test-data/goatbots/card-defs-small-5cards.json";
const auto path_goatbots_price_hist_small_5cards = "../../../test/test-data/goatbots/price-hist-small-5cards.json";
const auto path_mtgogetter_out_scryfall_small_5cards =
  "../../../test/test-data/mtgogetter-out/scryfall-small-5cards.json";
const auto path_trade_list_small_5cards = "../../../test/test-data/mtgo/Full Trade List-small-5cards.dek";


TEST_CASE("Parse small collection")
{
  SECTION("Small collection - 5 cards")
  {
    auto scryfall_vec = scryfall::ReadJsonVector(path_mtgogetter_out_scryfall_small_5cards);
    REQUIRE(scryfall_vec.has_value());
    CHECK(scryfall_vec.value().size() == 5);

    auto card_defs = goatbots::ReadJsonMap<goatbots::card_defs_map_t>(path_goatbots_card_defs_small_5cards);
    REQUIRE(card_defs.has_value());
    CHECK(card_defs.value().size() == 5);

    auto price_hist = goatbots::ReadJsonMap<goatbots::price_hist_map_t>(path_goatbots_price_hist_small_5cards);
    REQUIRE(price_hist.has_value());
    CHECK(price_hist.value().size() == 5);

    auto mtgo_cards = mtgo::xml::parse_dek_xml(path_trade_list_small_5cards);
    CHECK(mtgo_cards.size() == 5);

    auto mtgo_collection = mtgo::Collection(std::move(mtgo_cards));
    REQUIRE(mtgo_collection.Size() == 5);

    mtgo_collection.ExtractGoatbotsInfo(card_defs.value(), price_hist.value());
    mtgo_collection.PrettyPrint();
    mtgo_collection.ExtractScryfallInfo(std::move(scryfall_vec.value()));
    mtgo_collection.PrettyPrint();

    CHECK(mtgo_collection.TotalCards() == 457);

    auto pretty_json_str = mtgo_collection.ToJsonPretty();
    CHECK(mtgo_collection == mtgo::Collection(pretty_json_str));
  }

  const auto path_mtgogetter_out_scryfall_small_100cards =
    "../../../test/test-data/mtgogetter-out/scryfall-small-100cards.json";
  const auto path_trade_list_medium_3000cards = "../../../test/test-data/mtgo/Full Trade List-medium-3000cards.dek";
  const auto path_goatbots_card_defs_full = "../../../test/test-data/goatbots/card-definitions-2023-10-02-full.json";
  const auto path_goatbots_price_hist_full = "../../../test/test-data/goatbots/price-history-2023-10-02-full.json";


  SECTION("Medium collection - 3000 cards")
  {
    auto scryfall_vec = scryfall::ReadJsonVector(path_mtgogetter_out_scryfall_small_100cards);
    REQUIRE(scryfall_vec.has_value());
    CHECK(scryfall_vec.value().size() == 100);

    auto card_defs = goatbots::ReadJsonMap<goatbots::card_defs_map_t>(path_goatbots_card_defs_full);
    REQUIRE(card_defs.has_value());
    CHECK(card_defs.value().size() == 76070);

    auto price_hist = goatbots::ReadJsonMap<goatbots::price_hist_map_t>(path_goatbots_price_hist_full);
    REQUIRE(price_hist.has_value());
    CHECK(price_hist.value().size() == 76070);

    auto mtgo_cards = mtgo::xml::parse_dek_xml(path_trade_list_medium_3000cards);
    CHECK(mtgo_cards.size() == 3000);

    auto mtgo_collection = mtgo::Collection(std::move(mtgo_cards));
    REQUIRE(mtgo_collection.Size() == 3000);

    mtgo_collection.ExtractGoatbotsInfo(card_defs.value(), price_hist.value());
    mtgo_collection.ExtractScryfallInfo(std::move(scryfall_vec.value()));
    mtgo_collection.PrettyPrint();

    auto pretty_json_str = mtgo_collection.ToJsonPretty();
    CHECK(mtgo_collection == mtgo::Collection(pretty_json_str));
  }
}

// NOLINTEND