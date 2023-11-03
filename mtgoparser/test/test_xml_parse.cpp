#if _MSC_VER && !__INTEL_COMPILER
// On MSVC: Disable warning "discarding return value of function with 'nodiscard' attribute"
//  Because they warn on their own std::vector implementation, a warning that is discouraged by the standard...
#pragma warning(disable : 4834)
#endif
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <mtgoparser/mtgo.hpp>
#include <mtgoparser/mtgo/card.hpp>
#include <mtgoparser/mtgo/xml.hpp>

#include <cstddef>
#include <utility>
#include <vector>

// Goes to top of project and into the shared 'test/test-data' directory
const auto path_trade_list_small_5cards = "../../../test/test-data/mtgo/Full Trade List-small-5cards.dek";
const auto path_trade_list_small_50cards = "../../../test/test-data/mtgo/Full Trade List-small-50cards.dek";
const auto path_trade_list_small_500cards = "../../../test/test-data/mtgo/Full Trade List-small-500cards.dek";
const auto path_trade_list_medium_3000cards = "../../../test/test-data/mtgo/Full Trade List-medium-3000cards.dek";

TEST_CASE("Card structs can be deserialized from XML", "[cards_from_xml]")
{
  std::vector<mtgo::Card> cards = mtgo::xml::parse_dek_xml(path_trade_list_small_5cards);

  SECTION("Sanity tests - small 5 cards")
  {
    REQUIRE(cards.size() == 5);
    CHECK(cards.at(0).name_ == "Event Ticket");
    CHECK(cards[0].id_ == 1);
    CHECK(cards.at(0).quantity_ == 453);

    CHECK(cards.at(1).name_ == "Swamp");
    CHECK(cards.at(1).id_ == 235);
    CHECK(cards.at(1).quantity_ == 1);

    CHECK(cards.at(2).name_ == "Noble Hierarch");
    CHECK(cards.at(2).id_ == 31745);
    CHECK(cards.at(2).quantity_ == 1);

    CHECK(cards.at(3).name_ == "Black Lotus");
    CHECK(cards.at(3).id_ == 53155);
    CHECK(cards.at(3).quantity_ == 1);

    CHECK(cards.at(4).name_ == "Tranquil Cove");
    CHECK(cards.at(4).id_ == 110465);
    CHECK(cards.at(4).quantity_ == 1);
  }

  SECTION("Sanity checks on all full trade list files")
  {
    const std::pair<const char *, std::size_t> test_file_card_count_pair =
      GENERATE(std::make_pair(path_trade_list_small_5cards, 5),
        std::make_pair(path_trade_list_small_50cards, 50),
        std::make_pair(path_trade_list_small_500cards, 500),
        std::make_pair(path_trade_list_medium_3000cards, 3000));

    CHECK(mtgo::xml::parse_dek_xml(get<0>(test_file_card_count_pair)).size() == get<1>(test_file_card_count_pair));
  }
}


TEST_CASE("Deserialized cards throws with misuse", "[cards_from_xml]")
{
  std::vector<mtgo::Card> cards = mtgo::xml::parse_dek_xml(path_trade_list_small_5cards);

  SECTION("Throws out-of-bounds")
  {
    REQUIRE(cards.size() == 5);
    REQUIRE_THROWS(cards.at(6));
  }
}


TEST_CASE("MTGO collection")
{
  SECTION("Collection initialization and size")
  {
    SECTION("Small collection")
    {
      // From cards vector
      std::vector<mtgo::Card> cards = mtgo::xml::parse_dek_xml(path_trade_list_small_5cards);
      auto collection = mtgo::Collection(std::move(cards));
      REQUIRE(collection.Size() == 5);

      // From JSON string
      auto collection2 = mtgo::Collection(collection.ToJson());
      REQUIRE(collection2.Size() == collection.Size());

      // Total quantity
      CHECK(collection.TotalCards() == 457);// Hand counted :)
    }

    SECTION("Medium collection - 3000 cards")
    {
      // From cards vector
      std::vector<mtgo::Card> cards = mtgo::xml::parse_dek_xml(path_trade_list_medium_3000cards);
      auto collection = mtgo::Collection(std::move(cards));
      REQUIRE(collection.Size() == 3000);

      // From JSON string
      auto collection2 = mtgo::Collection(collection.ToJson());
      REQUIRE(collection2.Size() == collection.Size());

      // Total quantity
      CHECK(collection.TotalCards()
            == 8859);// Hand counted :) (with grep and this regex: `(?:Quantity=")(.*)(?=\" Sideboard)`)
    }
  }
}
