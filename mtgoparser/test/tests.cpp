#include <catch2/catch_test_macros.hpp>


// False positive on macos-12 GCC-13 with Release mode.
#if __GNUC__ && defined(__has_warning)
#define SUPPRESSING
#pragma GCC diagnostic push
#if __has_warning("-Warray-bounds")
#pragma GCC diagnostic ignored "-Warray-bounds"
#endif
#if __has_warning("-Wstringop-overread")
#pragma GCC diagnostic ignored "-Wstringop-overread"
#endif
#endif

#include <mtgoparser/mtgo.hpp>

#ifdef SUPPRESSING
#undef SUPPRESSING
#pragma GCC diagnostic pop
#endif


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
