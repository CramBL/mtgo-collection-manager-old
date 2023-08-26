#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>

#include <mtgoparser/mtgo.hpp>

const auto trade_list_file_path_small_5cards = "../../test/test-data/mtgo/Full Trade List-small-5cards.dek";
const auto trade_list_file_path_small_50cards = "../../test/test-data/mtgo/Full Trade List-small-50cards.dek";
const auto trade_list_file_path_small_500cards = "../../test/test-data/mtgo/Full Trade List-small-500cards.dek";
const auto trade_list_file_path_medium_3000cards = "../../test/test-data/mtgo/Full Trade List-medium-3000cards.dek";

TEST_CASE("parse_dek_xml", "[xml-bench]")
{

  REQUIRE(mtgo::xml::parse_dek_xml(trade_list_file_path_small_5cards).size() == 5);
  REQUIRE(mtgo::xml::parse_dek_xml(trade_list_file_path_small_50cards).size() == 50);
  REQUIRE(mtgo::xml::parse_dek_xml(trade_list_file_path_small_500cards).size() == 500);
  REQUIRE(mtgo::xml::parse_dek_xml(trade_list_file_path_medium_3000cards).size() == 3000);

  BENCHMARK("small - 5 cards") { return mtgo::xml::parse_dek_xml(trade_list_file_path_small_5cards); };
  BENCHMARK("small - 50 cards") { return mtgo::xml::parse_dek_xml(trade_list_file_path_small_50cards); };
  BENCHMARK("small - 500 cards") { return mtgo::xml::parse_dek_xml(trade_list_file_path_small_500cards); };
  BENCHMARK("medium - 3182 cards") { return mtgo::xml::parse_dek_xml(trade_list_file_path_medium_3000cards); };
}
