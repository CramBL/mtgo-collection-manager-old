// NOLINTBEGIN

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <fmt/core.h>

#include <mtgoparser/mtgo/csv.hpp>

#include <utility>

using Catch::Matchers::ContainsSubstring;


TEST_CASE("mtgo::csv::into_substr_vec")
{
  const std::string test_csv_data =
    R"(id,quantity,name,set,rarity,foil,2023-11-06T083944Z,2023-11-06T115147Z,2023-11-08T084732Z
120020,1,In the Darkness Bind Them,LTC,R,false,0.72;0.1,0.78;-,0.4;0.3
106729,1,Razorverge Thicket,ONE,R,false,1.1;0.9,2.0;2.1,0.9;-
106729,1,Razorverge Thicket,THR,R,false,-;-,2.0;2.1,0.9;-)";

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

TEST_CASE("mtgo::csv::str_to_floats")
{
  auto [a0, b0] = mtgo::csv::str_to_floats("0.72;0.1");
  INFO("a0: " << a0.value_or(-1.0f));
  INFO("b0: " << b0.value_or(-1.0f));
  REQUIRE(a0.has_value());
  REQUIRE(b0.has_value());
  CHECK(a0.value() == 0.72f);
  CHECK(b0.value() == 0.1f);

  auto [a01, b01] = mtgo::csv::str_to_floats("0.002;12.1");
  CHECK(a01.has_value());
  CHECK(b01.has_value());
  CHECK(a01.value() == 0.002f);
  CHECK(b01.value() == 12.1f);

  auto [a1, b1] = mtgo::csv::str_to_floats("0.72;-");
  CHECK(a1.has_value());
  CHECK_FALSE(b1.has_value());
  CHECK(a1.value() == 0.72f);

  auto [a2, b2] = mtgo::csv::str_to_floats("-;0.1");
  CHECK_FALSE(a2.has_value());
  CHECK(b2.has_value());
  CHECK(b2.value() == 0.1f);

  auto [a3, b3] = mtgo::csv::str_to_floats("-;-");
  CHECK_FALSE(a3.has_value());
  CHECK_FALSE(b3.has_value());

  // With more than two values
  auto [a4, b4] = mtgo::csv::str_to_floats("0.72;0.1;0.2");
  CHECK(a4.has_value());
  CHECK(b4.has_value());
  CHECK(a4.value() == 0.72f);
  CHECK(b4.value() == 0.1f);

  // With integer values
  auto [a5, b5] = mtgo::csv::str_to_floats("1;2");
  CHECK(a5.has_value());
  CHECK(b5.has_value());
  CHECK(a5.value() == 1.0f);
  CHECK(b5.value() == 2.0f);
}


TEST_CASE("mtgo::csv::into_substr_vec & mtgo::csv::str_to_floats")
{
  const std::string test_csv_data =
    R"(id,quantity,name,set,rarity,foil,2023-11-06T083944Z,2023-11-06T115147Z,2023-11-08T084732Z
120020,1,In the Darkness Bind Them,LTC,R,false,0.72;0.1,0.78;-,0.4;0.3
106729,1,Razorverge Thicket,ONE,R,false,1.1;0.9,2.0;2.1,0.9;-
106729,1,Razorverge Thicket,THR,R,false,-;-,2.0;2.1,0.9;-)";

  std::vector<std::string> rows = mtgo::csv::into_substr_vec(test_csv_data, '\n');
  REQUIRE(rows.size() == 4);

  auto headers = mtgo::csv::into_substr_vec(rows[0], ',');
  REQUIRE(headers.size() == 9);

  auto row1 = mtgo::csv::into_substr_vec(rows.at(1), ',');
  REQUIRE(row1.size() == 9);

  auto row2 = mtgo::csv::into_substr_vec(rows.at(2), ',');
  REQUIRE(row2.size() == 9);

  auto row3 = mtgo::csv::into_substr_vec(rows.at(3), ',');
  REQUIRE(row3.size() == 9);

  SECTION("Floating number parsing")
  {
    SECTION("Row 1")
    {
      auto [a0, b0] = mtgo::csv::str_to_floats(row1.at(6));
      CHECK(a0.has_value());
      CHECK(b0.has_value());
      CHECK(a0.value() == 0.72f);
      CHECK(b0.value() == 0.1f);

      auto [a1, b1] = mtgo::csv::str_to_floats(row1.at(7));
      CHECK(a1.has_value());
      CHECK_FALSE(b1.has_value());
      CHECK(a1.value() == 0.78f);

      auto [a2, b2] = mtgo::csv::str_to_floats(row1.at(8));
      CHECK(a2.has_value());
      CHECK(b2.has_value());
      CHECK(a2.value() == 0.4f);
      CHECK(b2.value() == 0.3f);
    }

    SECTION("Row 2")
    {
      for (std::size_t i = 6; i < 9; ++i) {
        auto [a, b] = mtgo::csv::str_to_floats(row2.at(i));
        if (i == 6) {
          CHECK(a.has_value());
          CHECK(b.has_value());
          CHECK(a.value() == 1.1f);
          CHECK(b.value() == 0.9f);
        } else if (i == 7) {
          CHECK(a.has_value());
          CHECK(b.has_value());
          CHECK(a.value() == 2.0f);
          CHECK(b.value() == 2.1f);
        } else if (i == 8) {
          CHECK(a.has_value());
          CHECK_FALSE(b.has_value());
          CHECK(a.value() == 0.9f);
        }
      }
    }
  }
}

// NOLINTEND