#include <catch2/catch_test_macros.hpp>

#include <mtgoparser/mtgo.hpp>

const auto trade_list_file_path = "../../test/test-data/Full Trade List-small.dek";

TEST_CASE("Card structs can be deserialized from XML", "[cards_from_xml]")
{
  std::vector<mtgo::Card> cards = mtgo::xml::parse_dek_xml(trade_list_file_path);

  SECTION("Sanity tests")
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
}


TEST_CASE("Deserialized cards throws with misuse", "[cards_from_xml]")
{
  std::vector<mtgo::Card> cards = mtgo::xml::parse_dek_xml(trade_list_file_path);

  SECTION("Throws out-of-bounds")
  {
    REQUIRE(cards.size() == 5);
    REQUIRE_THROWS(cards.at(6));
  }
}