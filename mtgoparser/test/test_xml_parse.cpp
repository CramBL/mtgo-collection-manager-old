#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <mtgoparser/mtgo.hpp>
#include <utility>

const auto trade_list_file_path_small_5cards = "../../test/test-data/mtgo/Full Trade List-small-5cards.dek";
const auto trade_list_file_path_small_50cards = "../../test/test-data/mtgo/Full Trade List-small-50cards.dek";
const auto trade_list_file_path_small_500cards = "../../test/test-data/mtgo/Full Trade List-small-500cards.dek";
const auto trade_list_file_path_medium_3000cards = "../../test/test-data/mtgo/Full Trade List-medium-3000cards.dek";

TEST_CASE("Card structs can be deserialized from XML", "[cards_from_xml]")
{
  std::vector<mtgo::Card> cards = mtgo::xml::parse_dek_xml(trade_list_file_path_small_5cards);

  SECTION("Sanity tests - small 5 cards")
  {
    REQUIRE(cards.size() == 5);
    CHECK(cards.at(0).name_ == "Event Ticket");
    CHECK(cards[0].id_ == "1");
    CHECK(cards.at(0).quantity_ == "453");

    CHECK(cards.at(1).name_ == "Swamp");
    CHECK(cards.at(1).id_ == "235");
    CHECK(cards.at(1).quantity_ == "1");

    CHECK(cards.at(2).name_ == "Noble Hierarch");
    CHECK(cards.at(2).id_ == "31745");
    CHECK(cards.at(2).quantity_ == "1");

    CHECK(cards.at(3).name_ == "Black Lotus");
    CHECK(cards.at(3).id_ == "53155");
    CHECK(cards.at(3).quantity_ == "1");

    CHECK(cards.at(4).name_ == "Tranquil Cove");
    CHECK(cards.at(4).id_ == "110465");
    CHECK(cards.at(4).quantity_ == "1");
  }

  SECTION("Sanity checks on all full trade list files")
  {
    const std::pair<const char *, std::size_t> test_file_card_count_pair =
      GENERATE(std::make_pair(trade_list_file_path_small_5cards, 5),
        std::make_pair(trade_list_file_path_small_50cards, 50),
        std::make_pair(trade_list_file_path_small_500cards, 500),
        std::make_pair(trade_list_file_path_medium_3000cards, 3000));

    CHECK(mtgo::xml::parse_dek_xml(get<0>(test_file_card_count_pair)).size() == get<1>(test_file_card_count_pair));
  }
}


TEST_CASE("Deserialized cards throws with misuse", "[cards_from_xml]")
{
  std::vector<mtgo::Card> cards = mtgo::xml::parse_dek_xml(trade_list_file_path_small_5cards);

  SECTION("Throws out-of-bounds")
  {
    REQUIRE(cards.size() == 5);
    REQUIRE_THROWS(cards.at(6));
  }
}