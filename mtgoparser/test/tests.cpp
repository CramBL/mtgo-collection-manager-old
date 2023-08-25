#include <catch2/catch_test_macros.hpp>

// False positive on macos-12 GCC-13 with Release mode.
#pragma GCC diagnostic ignored "-Warray-bounds"
#include <mtgoparser/mtgo.hpp>


TEST_CASE("Card structs can be deserialized from XML", "[cards_from_xml]")
{
  std::vector<mtgo::Card> cards = mtgo::xml::parse_dek_xml("../../test/test-data/Full Trade List-small.dek");

  SECTION("Sanity tests")
  {
    REQUIRE(cards.size() == 5);
    CHECK(cards.at(0).name_ == "Event Ticket");
    CHECK(cards[0].id_ == "1");
  }
}
