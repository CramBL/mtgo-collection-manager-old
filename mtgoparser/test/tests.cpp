// False positive on macos-12 GCC-13 with Release mode.
#pragma message "Compiling tests.cpp"
#if (defined(__GNUC__) || defined(__clang__)) && defined(__has_warning)
#pragma message "GNUC or CLANG and __has_warning defined -> suppressing selected warnings with false positives"
#if defined(__GNUC__)
#pragma message "GNUC defined"
#endif
#if defined(__clang__)
#pragma message "clang defined"
#endif
#if defined(__has_warning)
#pragma message "__has_warning defined"
#endif

#define SUPPRESSING
#pragma GCC diagnostic push
#if __has_warning("-Warray-bounds")
#pragma message "Disabling warning: -Warray-bounds"
#pragma GCC diagnostic ignored "-Warray-bounds"
#endif
#if __has_warning("-Wstringop-overread")
#pragma message "Disabling warning: -Wstringop-overread"
#pragma GCC diagnostic ignored "-Wstringop-overread"
#endif
#endif

#include <catch2/catch_test_macros.hpp>
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


#ifdef SUPPRESSING
#undef SUPPRESSING
#pragma message "Reverting local warning suppressions"
#pragma GCC diagnostic pop
#endif