// NOLINTBEGIN
#if _MSC_VER && !__INTEL_COMPILER
// On MSVC: Disable warning "discarding return value of function with 'nodiscard' attribute"
//  Because they warn on their own std::vector implementation, a warning that is discouraged by the standard...
#pragma warning(disable : 4834)
#endif
#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>

#include <mtgoparser/mtgo.hpp>
#include <mtgoparser/mtgo/card.hpp>

#include <algorithm>
#ifndef __APPLE__
#include <execution>
#endif
#include <numeric>
#include <string>
#include <utility>
#include <vector>

// Goes to top of project and into the shared 'test/test-data' directory
const auto path_trade_list_small_5cards = "../../../test/test-data/mtgo/Full Trade List-small-5cards.dek";
const auto path_trade_list_small_50cards = "../../../test/test-data/mtgo/Full Trade List-small-50cards.dek";
const auto path_trade_list_small_500cards = "../../../test/test-data/mtgo/Full Trade List-small-500cards.dek";
const auto path_trade_list_medium_3000cards = "../../../test/test-data/mtgo/Full Trade List-medium-3000cards.dek";

// Hidden with . prefix to avoid running benchmark in every CI run
// To run hidden tests specify the [.] tag i.e. ./build/test/benchmark_xml_parse [.]
TEST_CASE("parse_dek_xml", "[.xml-parse-bench]")// .(dot) prefix hides the test by default
{
  REQUIRE(mtgo::xml::parse_dek_xml(path_trade_list_small_5cards).size() == 5);
  REQUIRE(mtgo::xml::parse_dek_xml(path_trade_list_small_50cards).size() == 50);
  REQUIRE(mtgo::xml::parse_dek_xml(path_trade_list_small_500cards).size() == 500);
  REQUIRE(mtgo::xml::parse_dek_xml(path_trade_list_medium_3000cards).size() == 3000);

  BENCHMARK("small - 5 cards") { return mtgo::xml::parse_dek_xml(path_trade_list_small_5cards); };
  BENCHMARK("small - 50 cards") { return mtgo::xml::parse_dek_xml(path_trade_list_small_50cards); };
  BENCHMARK("small - 500 cards") { return mtgo::xml::parse_dek_xml(path_trade_list_small_500cards); };
  BENCHMARK("medium - 3000 cards") { return mtgo::xml::parse_dek_xml(path_trade_list_medium_3000cards); };
}

#ifndef __APPLE__


/***                                                                                              ***
 *** No longer valid, just here for reference. Replace the next time something similar is needed. ***
 ***                                                                                              ***/
// Hidden with . prefix to avoid running benchmark in every CI run
// To run hidden tests specify the [.] tag i.e. ./build/test/benchmark_xml_parse [.]
// TEST_CASE("mtgo::collection parse cards parallelizable",
//   "[.collection-analyze-bench]")// .(dot) prefix hides the test by default
// {
//   // Parse quantity from string to uint32_t
//   // Test the performance of parallelizing this operation

//   // First get vector of 3000 cards
//   std::vector<mtgo::Card> cards_ = mtgo::xml::parse_dek_xml(path_trade_list_medium_3000cards);
//   const auto expected_card_quantity = 8859;

//   // Produce vector of quantities
//   std::vector<uint16_t> card_quantity_tmp(cards_.size(), 0);

//   BENCHMARK("Transform - sequential")
//   {
//     std::transform(std::execution::seq,
//       cards_.begin(),
//       cards_.end(),
//       card_quantity_tmp.begin(),
//       [](const mtgo::Card &c) -> uint16_t { return static_cast<uint16_t>(std::stoul(c.quantity_)); });

//     REQUIRE(card_quantity_tmp.front() == 391);
//     REQUIRE(card_quantity_tmp.back() == 1);

//     auto total_quantity = std::reduce(
//       std::execution::seq, card_quantity_tmp.begin(), card_quantity_tmp.end(), 0, [](const auto &a, const auto &b) {
//         return a + b;
//       });

//     REQUIRE(total_quantity == expected_card_quantity);

//     return total_quantity;
//   };

//   BENCHMARK("Transform - parallel")
//   {
//     std::transform(std::execution::par,
//       cards_.begin(),
//       cards_.end(),
//       card_quantity_tmp.begin(),
//       [](const mtgo::Card &c) -> uint16_t { return static_cast<uint16_t>(std::stoul(c.quantity_)); });
//     REQUIRE(card_quantity_tmp.front() == 391);
//     REQUIRE(card_quantity_tmp.back() == 1);

//     auto total_quantity = std::reduce(
//       std::execution::par, card_quantity_tmp.begin(), card_quantity_tmp.end(), 0, [](const auto &a, const auto &b) {
//         return a + b;
//       });

//     REQUIRE(total_quantity == expected_card_quantity);

//     return total_quantity;
//   };

// BENCHMARK("Transform - unsequenced parallel")
// {
//   std::transform(std::execution::par_unseq,
//     cards_.begin(),
//     cards_.end(),
//     card_quantity_tmp.begin(),
//     [](const mtgo::Card &c) -> uint16_t { return static_cast<uint16_t>(std::stoul(c.quantity_)); });
//   REQUIRE(card_quantity_tmp.front() == 391);
//   REQUIRE(card_quantity_tmp.back() == 1);

//   auto total_quantity = std::reduce(std::execution::par_unseq,
//     card_quantity_tmp.begin(),
//     card_quantity_tmp.end(),
//     0,
//     [](const auto &a, const auto &b) { return a + b; });

//   REQUIRE(total_quantity == expected_card_quantity);

//   return total_quantity;
// };
//}

#endif

// NOLINTEND