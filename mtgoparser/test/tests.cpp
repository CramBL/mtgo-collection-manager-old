// NOLINTBEGIN

#include "mtgoparser/clap/option.hpp"
#include "mtgoparser/io.hpp"
#include <catch2/catch_test_macros.hpp>
#include <fmt/core.h>
#include <mtgoparser/clap.hpp>
#include <mtgoparser/io.hpp>
#include <mtgoparser/mtg.hpp>
#include <mtgoparser/mtgo/card.hpp>
#include <mtgoparser/util.hpp>


#include <string_view>
#include <utility>

using clap::Opt::Flag;
using clap::Opt::NeedValue;

constinit auto static_clap = clap::Clap<1, 0>(clap::OptionArray<1>(clap::Option("--version", Flag)));


TEST_CASE("Test basic CLAP")
{

  char argv0[] = "mtgo_preprocessor";
  char argv1[] = "--version";

  char *argv[] = { argv0, argv1 };
  int argc = 2;

  std::vector<std::string_view> arg_vec{ argv + 1, argv + argc };

  SECTION("Dynamically initialized - Show version")
  {
    auto clap = clap::Clap<1, 0>(clap::OptionArray<1>(clap::Option("--version", Flag)));
    fmt::print("Options are:\n");
    clap.PrintOptions();

    CHECK(clap.Parse(arg_vec) == 0);
    fmt::print("Arguments are:\n");
    clap.PrintArgs();
  }

  SECTION("Static initialized - show version")
  {
    fmt::print("Parsing arguments with constinit Clap\n");
    CHECK(static_clap.Parse(arg_vec) == 0);
    fmt::print("Arguments are:\n");
    static_clap.PrintArgs();
  }

  SECTION("Alias version cmd - Show version")
  {

    auto clap_alias_version = clap::Clap<1, 0>(clap::OptionArray<1>(clap::Option("--version", Flag, "-V")));

    CHECK(clap_alias_version.Parse(arg_vec) == 0);

    fmt::print("Arguments are:\n");
    clap_alias_version.PrintArgs();

    CHECK(clap_alias_version.FlagSet("--version"));
    CHECK(clap_alias_version.FlagSet("-V"));
  }
}

TEST_CASE("Test CLAP with options and values")
{

  char argv0[] = "mtgo_preprocessor";
  char arg_version[] = "--version";
  char arg_save_as[] = "--save-as";
  char arg_save_as_val[] = "saved.txt";

  SECTION("test save as option value")
  {
    char *argv[] = { argv0, arg_save_as, arg_save_as_val };
    int argc = 3;

    std::vector<std::string_view> arg_vec{ argv + 1, argv + argc };

    auto clap = clap::Clap<2, 0>(
      clap::OptionArray<2>(clap::Option("--version", Flag, "-V"), clap::Option("--save-as", NeedValue, "-s")));


    CHECK(clap.Parse(arg_vec) == 0);
    fmt::print("Got args:\n");
    clap.PrintArgs();

    CHECK(clap.OptionValue("--save-as").value() == arg_save_as_val);
    CHECK(clap.OptionValue("-s").value() == arg_save_as_val);
    CHECK(clap.FlagSet("--version") == false);
    CHECK(clap.FlagSet("-V") == false);
  }

  SECTION("Argument validation catches errors")
  {
    constexpr auto version_option = clap::Option("--version", Flag, "-V");
    constexpr auto save_as_option = clap::Option("--save-as", NeedValue, "-s");
    constexpr auto opt_arr = clap::OptionArray<2>(version_option, save_as_option);
    auto clap = clap::Clap<2, 0>(opt_arr);


    SECTION("Missing option value - end of args")
    {
      char *argv[] = { argv0, arg_save_as };
      int argc = 2;
      std::vector<std::string_view> arg_vec{ argv + 1, argv + argc };


      fmt::print("Got args:\n");
      fmt::print("Should fail as --save-as doesn't have a value provided\n");
      CHECK(clap.Parse(arg_vec) != 0);
    }

    SECTION("Missing option value - next option instead of value")
    {
      char *argv[] = { argv0, arg_save_as, arg_version };
      int argc = 3;
      std::vector<std::string_view> arg_vec{ argv + 1, argv + argc };

      fmt::print("Got args:\n");
      fmt::print(
        "Should fail as --save-as doesn't have a value provided, instead it's followed by the --version option\n");
      CHECK(clap.Parse(arg_vec) != 0);
    }
  }
}

TEST_CASE("MTGO card - Initialize and use of")
{

  SECTION("Initialize")
  {
    // Test constructors, assignments, initializations with different types
    unsigned short int id_1 = 1;
    mtgo::Card mtgo_card = mtgo::Card(id_1, util::sv_to_uint<uint16_t>("1").value_or(123), "name", "set", "Common");
    CHECK(mtgo_card.id_ == 1);
    CHECK(mtgo_card.quantity_ == 1);
    CHECK(mtgo_card.name_ == "name");
    CHECK(mtgo_card.set_ == "set");
    CHECK(mtgo_card.rarity_ == mtg::Rarity::Common);
    CHECK(mtgo_card.foil_ == false);
    CHECK(mtgo_card.goatbots_price_ == 0);
    REQUIRE(mtgo_card.scryfall_price_.has_value() == false);
    REQUIRE(mtgo_card.scryfall_price_ == std::nullopt);

    unsigned int id2 = 1;
    mtgo::Card mtgo_card2 =
      mtgo::Card(id2, util::sv_to_uint<uint16_t>("1").value_or(9), "name", "set", "C", true, 1.0, 2.0);
    CHECK(mtgo_card2.id_ == 1);
    CHECK(mtgo_card2.quantity_ == 1);
    CHECK(mtgo_card2.name_ == "name");
    CHECK(mtgo_card2.set_ == "set");
    CHECK(mtgo_card2.rarity_ == mtg::Rarity::Common);
    CHECK(mtgo_card2.foil_ == true);
    CHECK(mtgo_card2.goatbots_price_ == 1.0);
    REQUIRE(mtgo_card2.scryfall_price_.has_value());
    REQUIRE(mtgo_card2.scryfall_price_.value() == 2.0);

    CHECK(mtgo_card != mtgo_card2);

    // Check initialization from string_view
    uint32_t id = 1;
    std::string_view quantity = "1";
    std::string_view name = "name";
    std::string_view set = "set";
    std::string_view rarity = "common";
    mtgo::Card mtgo_card3 = mtgo::Card(id, util::sv_to_uint<uint16_t>(quantity).value_or(0), name, set, rarity);

    // check equality with mtgo_card2
    CHECK(mtgo_card3 != mtgo_card2);
    CHECK(mtgo_card3 == mtgo_card);

    // Check initialization from string
    std::string name_str = "name";
    std::string set_str = "set";
    std::string rarity_str = "COMMON";
    uint32_t id4 = 1;
    uint16_t quant4 = 1;
    mtgo::Card mtgo_card4 = mtgo::Card(id4, quant4, name_str, set_str, rarity_str);

    // check equality with mtgo_card
    CHECK(mtgo_card4 == mtgo_card);
    CHECK(mtgo_card4 == mtgo_card3);
    CHECK(mtgo_card4 != mtgo_card2);
  }

  SECTION("Card Move semantics")
  {
    // Test move constructors and move assignment

    uint16_t ids = 1;
    uint16_t quantities = 1;
    mtgo::Card mtgo_card = mtgo::Card(ids, quantities, "name", "set", "common", true, 1.0, 2.0);
    mtgo::Card mtgo_card2 = mtgo::Card(ids, quantities, "name", "set", "common", true, 1.0, 2.0);

    // Move constructor
    mtgo::Card mtgo_card3(std::move(mtgo_card));
    CHECK(mtgo_card3 == mtgo_card2);
    // Check that mtgo_card is now invalid (commented out as it triggered warning in CI)
    // CHECK(mtgo_card.id_ == "");// Access of moved value

    // Move assignment
    uint16_t id_tmp = 2;
    auto mtgo_card_tmp = mtgo::Card(id_tmp, quantities, "name", "set", "common", true, 1.0, 2.0);
    mtgo_card3 = std::move(mtgo_card_tmp);
    CHECK(mtgo_card3 != mtgo_card2);// ID should differ
    // Check that mtgo_card_tmp is now invalid (commented out as it triggered warning in CI)
    // CHECK(mtgo_card_tmp.id_ == ""); // Access of moved value (compiler warning)
  }
}

TEST_CASE("Command struct")
{
  // Command with no aliases
  constexpr clap::Command cmd0{ "my-cmd", false };
  CHECK(cmd0.name_ == "my-cmd");
  CHECK(cmd0.is_flag_ == false);

  // with alias
  constexpr clap::Command cmd1{ "my-cmd1", false };
  CHECK(cmd1.name_ == "my-cmd1");
  CHECK(cmd1.is_flag_ == false);

  // With multiple aliases
  constexpr clap::Command cmd2{ "my-cmd2", true };
  CHECK(cmd2.name_ == "my-cmd2");
  CHECK(cmd2.is_flag_ == true);

  // They can fit in same cmd array
  constexpr std::array<clap::Command, 3> cmd_arr = { cmd0, cmd1, cmd2 };
  REQUIRE(cmd_arr.at(0).name_ == cmd0.name_);
  CHECK(cmd0.is_flag_ == false);

  REQUIRE(cmd_arr.at(2).name_ == "my-cmd2");
  REQUIRE(cmd_arr.at(2).is_flag_ == true);

  constexpr clap::CommandArray<3> my_cmd_arr{ cmd0, cmd1, cmd2 };
  constexpr auto arr_sz = my_cmd_arr.size();// Circumvent CPP check warning: [knownConditionTrueFalse]
  REQUIRE(arr_sz == 3);
  CHECK(my_cmd_arr.find("my-cmd2").has_value());
  CHECK(my_cmd_arr.find("my-cmd1").value().name_ == "my-cmd1");
  CHECK(my_cmd_arr.find("my-cmd1").value().is_flag_ == false);
}

TEST_CASE("Option struct")
{
  constexpr clap::Option opt{ "--my-option", Flag };
  constexpr clap::Option opt_w_alias("--my-option", Flag, "--my-alias");

  constexpr bool opt_has_alias = opt.has_alias();
  REQUIRE(opt_has_alias == false);

  constexpr bool opt_w_alias_has_alias = opt_w_alias.has_alias();
  REQUIRE(opt_w_alias_has_alias == true);

  constexpr clap::OptionArray<2> opt_arr{ opt, opt_w_alias };

  constexpr auto arr_sz = opt_arr.size();
  CHECK(arr_sz == 2);

  CHECK(opt_arr.find("--my-option").has_value() == true);
  CHECK(opt_arr.find("--my-alias").has_value() == true);

  auto found_opt = opt_arr.find("--my-alias");
  REQUIRE(found_opt.has_value() == true);
  CHECK(found_opt.value().name_ == "--my-option");
}

TEST_CASE("Parse state_log.toml")
{
  const auto path_state_log = "../../../test/test-data/mtgogetter-out/state_log.toml";
  auto state_log = io_util::read_state_log(path_state_log);
  std::string_view title = state_log["Title"].value_or("");

  INFO("state_log has title: " << title);
  CHECK(title == "log for MTGO Getter state, such as updated_at timestamps");

  // Check goatbots values
  SECTION("Goatbots state_log data")
  {

    std::optional<toml::date_time> card_defs_updated_at =
      state_log["goatbots"]["Card_definitions_updated_at"].value<toml::date_time>();
    REQUIRE(card_defs_updated_at.has_value());

    INFO("state_log.goatbots.Card_definitions_updated_at: " << card_defs_updated_at.value());
    CHECK(
      card_defs_updated_at.value() == toml::date_time{ { 2023, 10, 21 }, { 22, 29, 53 }, {} });// "2023-10-21T22:29:53Z"

    std::optional<toml::date_time> prices_updated_at =
      state_log["goatbots"]["Prices_updated_at"].value<toml::date_time>();
    REQUIRE(prices_updated_at.has_value());
    INFO("state_log.goatbots.Prices_updated_at: " << prices_updated_at.value());
    CHECK(
      prices_updated_at.value() == toml::date_time{ { 2023, 10, 14 }, { 15, 24, 21 }, {} });// "2023-10-14T15:24:21Z"
  }

  SECTION("Scryfall state_log data")
  {
    // check scryfall values
    std::optional<toml::date_time> bulk_updated_at =
      state_log["scryfall"]["Bulk_data_updated_at"].value<toml::date_time>();
    REQUIRE(bulk_updated_at.has_value());
    INFO("state_log.scryfall.Bulk_data_updated_at: " << bulk_updated_at.value());
    CHECK(bulk_updated_at.value() == toml::date_time{ { 1970, 1, 1 }, { 0, 0, 0 }, {} });//"1970-01-01T00:00:00Z"

    // check scryfall next released set
    std::string_view next_released_set_name = state_log["scryfall"]["Next_released_mtgo_set"]["Name"].value_or("");
    CHECK(next_released_set_name == "The Lost Caverns of Ixalan");

    std::string_view next_released_set_date_str =
      state_log["scryfall"]["Next_released_mtgo_set"]["Released_at"].value_or("");
    CHECK(next_released_set_date_str == "2023-12-11");

    std::string_view next_released_set_mtgo_code =
      state_log["scryfall"]["Next_released_mtgo_set"]["Mtgo_code"].value_or("");
    CHECK(next_released_set_mtgo_code == "lci");
  }

  SECTION("Write to state_log")
  {
    // Pretend we found the mtgo_code in the card defs, now we should empty the "next_released_mtgo_set" fields
    toml::value<std::string> *name = state_log["scryfall"]["Next_released_mtgo_set"]["Name"].as_string();
    *name = "";
    toml::value<std::string> *released_at = state_log["scryfall"]["Next_released_mtgo_set"]["Released_at"].as_string();
    *released_at = "";
    toml::value<std::string> *mtgo_code = state_log["scryfall"]["Next_released_mtgo_set"]["Mtgo_code"].as_string();
    *mtgo_code = "";

    std::string name_str = state_log["scryfall"]["Next_released_mtgo_set"]["Name"].value_or("error");
    CHECK(name_str == "");

    std::string released_at_str = state_log["scryfall"]["Next_released_mtgo_set"]["Released_at"].value_or("error");
    CHECK(released_at_str == "");

    std::string mtgo_code_str = state_log["scryfall"]["Next_released_mtgo_set"]["Mtgo_code"].value_or("error");
    CHECK(mtgo_code_str == "");

    INFO("State log:\n" << state_log << '\n');

    SECTION("TOML File operations")
    {
      std::ofstream test_state_log_file("test_tmp_state_log.toml");
      REQUIRE(test_state_log_file.is_open());

      if (test_state_log_file.is_open()) {
        INFO("test_tmp_state_log.toml opened");
        test_state_log_file << state_log;
        test_state_log_file.close();
      } else {
        FAIL("Opening file for writing failed.");
      }


      std::ifstream newly_open_test_state_log_file("test_tmp_state_log.toml");
      std::string line{};
      REQUIRE(newly_open_test_state_log_file.is_open());
      if (newly_open_test_state_log_file.is_open()) {
        while (std::getline(newly_open_test_state_log_file, line)) { INFO("Line from file: " << line); }
        newly_open_test_state_log_file.close();
      } else {
        FAIL("Opening file for reading failed.");
      }
    }
  }
}

// NOLINTEND