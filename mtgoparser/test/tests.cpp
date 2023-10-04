// NOLINTBEGIN
#include <catch2/catch_test_macros.hpp>
#include <mtgoparser/clap.hpp>
#include <mtgoparser/mtg.hpp>
#include <mtgoparser/mtgo/card.hpp>
#include <mtgoparser/util.hpp>
#include <utility>


constinit auto static_clap = clap::Clap<1, 0>(clap::OptionArray<1>(clap::Option("--version", true)));


TEST_CASE("Test basic CLAP")
{

  char argv0[] = "mtgo_preprocessor";
  char argv1[] = "--version";

  char *argv[] = { argv0, argv1 };
  int argc = 2;

  SECTION("Dynamically initialized - Show version")
  {
    auto clap = clap::Clap<1, 0>(clap::OptionArray<1>(clap::Option("--version", true)));
    fmt::print("Options are:\n");
    clap.PrintOptions();

    CHECK(clap.Parse(argc, argv) == 0);
    fmt::print("Arguments are:\n");
    clap.PrintArgs();
  }

  SECTION("Static initialized - show version")
  {
    fmt::print("Parsing arguments with constinit Clap\n");
    CHECK(static_clap.Parse(argc, argv) == 0);
    fmt::print("Arguments are:\n");
    static_clap.PrintArgs();
  }

  SECTION("Alias version cmd - Show version")
  {

    auto clap_alias_version = clap::Clap<1, 0>(clap::OptionArray<1>(clap::Option("--version", true, "-V")));

    CHECK(clap_alias_version.Parse(argc, argv) == 0);

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

    auto clap = clap::Clap<2, 0>(
      clap::OptionArray<2>(clap::Option("--version", true, "-V"), clap::Option("--save-as", false, "-s")));


    CHECK(clap.Parse(argc, argv) == 0);
    fmt::print("Got args:\n");
    clap.PrintArgs();

    CHECK(clap.OptionValue("--save-as").value() == arg_save_as_val);
    CHECK(clap.OptionValue("-s").value() == arg_save_as_val);
    CHECK(clap.FlagSet("--version") == false);
    CHECK(clap.FlagSet("-V") == false);
  }

  SECTION("Argument validation catches errors")
  {
    constexpr auto version_option = clap::Option("--version", true, "-V");
    constexpr auto save_as_option = clap::Option("--save-as", false, "-s");
    constexpr auto opt_arr = clap::OptionArray<2>(version_option, save_as_option);
    auto clap = clap::Clap<2, 0>(opt_arr);


    SECTION("Missing option value - end of args")
    {
      char *argv[] = { argv0, arg_save_as };
      int argc = 2;
      fmt::print("Got args:\n");
      fmt::print("Should fail as --save-as doesn't have a value provided\n");
      CHECK(clap.Parse(argc, argv) != 0);
    }

    SECTION("Missing option value - next option instead of value")
    {
      char *argv[] = { argv0, arg_save_as, arg_version };
      int argc = 3;
      fmt::print("Got args:\n");
      fmt::print(
        "Should fail as --save-as doesn't have a value provided, instead it's followed by the --version option\n");
      CHECK(clap.Parse(argc, argv) != 0);
    }
  }
}

TEST_CASE("MTGO card - Initialize and use of")
{

  SECTION("Initialize")
  {
    // Test constructors, assignments, initializations with different types
    mtgo::Card mtgo_card = mtgo::Card(1, util::sv_to_uint<uint16_t>("1").value_or(123), "name", "set", "Common");
    CHECK(mtgo_card.id_ == 1);
    CHECK(mtgo_card.quantity_ == 1);
    CHECK(mtgo_card.name_ == "name");
    CHECK(mtgo_card.set_ == "set");
    CHECK(mtgo_card.rarity_ == mtg::Rarity::Common);
    CHECK(mtgo_card.foil_ == false);
    CHECK(mtgo_card.goatbots_price_ == 0);
    REQUIRE(mtgo_card.scryfall_price_.has_value() == false);
    REQUIRE(mtgo_card.scryfall_price_ == std::nullopt);

    mtgo::Card mtgo_card2 =
      mtgo::Card(1, util::sv_to_uint<uint16_t>("1").value_or(9), "name", "set", "C", true, 1.0, 2.0);
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
    std::string id_str = "1";
    std::string quantity_str = "1";
    std::string name_str = "name";
    std::string set_str = "set";
    std::string rarity_str = "COMMON";
    mtgo::Card mtgo_card4 = mtgo::Card(util::sv_to_uint<uint32_t>(id_str).value_or(0),
      util::sv_to_uint<uint16_t>(quantity_str).value_or(123),
      name_str,
      set_str,
      rarity_str);

    // check equality with mtgo_card
    CHECK(mtgo_card4 == mtgo_card);
    CHECK(mtgo_card4 == mtgo_card3);
    CHECK(mtgo_card4 != mtgo_card2);
  }

  SECTION("Card Move semantics")
  {
    // Test move constructors and move assignment

    mtgo::Card mtgo_card = mtgo::Card(1, 1, "name", "set", "common", true, 1.0, 2.0);
    mtgo::Card mtgo_card2 = mtgo::Card(1, 1, "name", "set", "common", true, 1.0, 2.0);

    // Move constructor
    mtgo::Card mtgo_card3(std::move(mtgo_card));
    CHECK(mtgo_card3 == mtgo_card2);
    // Check that mtgo_card is now invalid (commented out as it triggered warning in CI)
    // CHECK(mtgo_card.id_ == "");// Access of moved value

    // Move assignment
    auto mtgo_card_tmp = mtgo::Card(2, 1, "name", "set", "common", true, 1.0, 2.0);
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
  constexpr clap::Option opt{ "--my-option", true };
  constexpr clap::Option opt_w_alias("--my-option", true, "--my-alias");

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

// NOLINTEND