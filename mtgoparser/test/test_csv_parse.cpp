// NOLINTBEGIN

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <fmt/core.h>

#include <mtgoparser/mtgo/csv.hpp>

#include <utility>

using Catch::Matchers::ContainsSubstring;

const std::string test_csv_data =
  R"(id,quantity,name,set,rarity,foil,2023-11-06T083944Z,2023-11-06T115147Z,2023-11-08T084732Z
120020,1,In the Darkness Bind Them,LTC,R,false,0.72;0.1,0.78;-,0.4;0.3
106729,1,Razorverge Thicket,ONE,R,false,1.1;0.9,2.0;2.1,0.9;-
106729,1,Razorverge Thicket,THR,R,false,-;-,2.0;2.1,0.9;-)";

TEST_CASE("mtgo::csv")
{
  std::vector<std::string> rows = mtgo::csv::into_substr_vec(test_csv_data, '\n');
  REQUIRE(rows.size() == 4);

  CHECK(rows.at(0) == "id,quantity,name,set,rarity,foil,2023-11-06T083944Z,2023-11-06T115147Z,2023-11-08T084732Z");
  CHECK(rows.at(1) == "120020,1,In the Darkness Bind Them,LTC,R,false,0.72;0.1,0.78;-,0.4;0.3");
  CHECK(rows.at(2) == "106729,1,Razorverge Thicket,ONE,R,false,1.1;0.9,2.0;2.1,0.9;-");
  CHECK(rows.at(3) == "106729,1,Razorverge Thicket,THR,R,false,-;-,2.0;2.1,0.9;-");


  auto headers = mtgo::csv::into_substr_vec(rows[0], ',');
  REQUIRE(headers.size() == 9);

  CHECK(headers.at(0) == "id");
  CHECK(headers.at(1) == "quantity");
  CHECK(headers.at(2) == "name");
  CHECK(headers.at(3) == "set");
  CHECK(headers.at(4) == "rarity");
  CHECK(headers.at(5) == "foil");
  CHECK(headers.at(6) == "2023-11-06T083944Z");
  CHECK(headers.at(7) == "2023-11-06T115147Z");
  CHECK(headers.at(8) == "2023-11-08T084732Z");
}

// NOLINTEND