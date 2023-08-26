#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>

#include <mtgoparser/mtgo.hpp>

const auto path_trade_list_small_5cards = "../../test/test-data/mtgo/Full Trade List-small-5cards.dek";
const auto path_trade_list_small_50cards = "../../test/test-data/mtgo/Full Trade List-small-50cards.dek";
const auto path_trade_list_small_500cards = "../../test/test-data/mtgo/Full Trade List-small-500cards.dek";
const auto path_trade_list_medium_3000cards = "../../test/test-data/mtgo/Full Trade List-medium-3000cards.dek";

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
