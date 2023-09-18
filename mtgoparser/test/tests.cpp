// NOLINTBEGIN
#include <catch2/catch_test_macros.hpp>
#include <mtgoparser/clap.hpp>
#include <mtgoparser/mtgo/card.hpp>
#include <utility>

constinit auto static_clap = clap::Clap<1>(std::make_pair("--version", false));

TEST_CASE("Test basic CLAP")
{

  char argv0[] = "mtgo_preprocessor";
  char argv1[] = "--version";

  char *argv[] = { argv0, argv1 };
  int argc = 2;

  SECTION("Dynamically initialized - Show version")
  {
    auto clap = clap::Clap<1>(std::make_pair("--version", false));
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
    auto clap_alias_version = clap::Clap<2>(std::make_pair("--version", false), std::make_pair("-V", false));
    CHECK(clap_alias_version.Parse(argc, argv) == 0);

    fmt::print("Arguments are:\n");
    clap_alias_version.PrintArgs();

    CHECK(clap_alias_version.FlagSet("--version"));
    CHECK(clap_alias_version.FlagSet("--version", "-V"));
    CHECK(!clap_alias_version.FlagSet("-V"));
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

    auto clap = clap::Clap<4>(std::make_pair("--version", false),
      std::make_pair("-V", false),
      std::make_pair("--save-as", true),
      std::make_pair("-s", true));

    CHECK(clap.Parse(argc, argv) == 0);
    fmt::print("Got args:\n");
    clap.PrintArgs();

    CHECK(clap.OptionValue("--save-as", "-s").value() == arg_save_as_val);
    CHECK(clap.FlagSet("--version", "-V") == false);
  }

  SECTION("Argument validation catches errors")
  {

    auto clap = clap::Clap<4>(std::make_pair("--version", false),
      std::make_pair("-V", false),
      std::make_pair("--save-as", true),
      std::make_pair("-s", true));

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
    mtgo::Card mtgo_card = mtgo::Card("1", "1", "name", "set", "rarity");
    CHECK(mtgo_card.id_ == "1");
    CHECK(mtgo_card.quantity_ == "1");
    CHECK(mtgo_card.name_ == "name");
    CHECK(mtgo_card.set_ == "set");
    CHECK(mtgo_card.rarity_ == "rarity");
    CHECK(mtgo_card.foil_ == false);
    CHECK(mtgo_card.goatbots_price_ == 0);
    REQUIRE(mtgo_card.scryfall_price_ == 0);

    mtgo::Card mtgo_card2 = mtgo::Card("1", "1", "name", "set", "rarity", true, 1.0, 2.0);
    CHECK(mtgo_card2.id_ == "1");
    CHECK(mtgo_card2.quantity_ == "1");
    CHECK(mtgo_card2.name_ == "name");
    CHECK(mtgo_card2.set_ == "set");
    CHECK(mtgo_card2.rarity_ == "rarity");
    CHECK(mtgo_card2.foil_ == true);
    CHECK(mtgo_card2.goatbots_price_ == 1.0);
    REQUIRE(mtgo_card2.scryfall_price_ == 2.0);

    CHECK(mtgo_card != mtgo_card2);

    // Check initialization from string_view
    std::string_view id = "1";
    std::string_view quantity = "1";
    std::string_view name = "name";
    std::string_view set = "set";
    std::string_view rarity = "rarity";
    mtgo::Card mtgo_card3 = mtgo::Card(id, quantity, name, set, rarity);

    // check equality with mtgo_card2
    CHECK(mtgo_card3 != mtgo_card2);
    CHECK(mtgo_card3 == mtgo_card);

    // Check initialization from string
    std::string id_str = "1";
    std::string quantity_str = "1";
    std::string name_str = "name";
    std::string set_str = "set";
    std::string rarity_str = "rarity";
    mtgo::Card mtgo_card4 = mtgo::Card(id_str, quantity_str, name_str, set_str, rarity_str);

    // check equality with mtgo_card
    CHECK(mtgo_card4 == mtgo_card);
    CHECK(mtgo_card4 == mtgo_card3);
    CHECK(mtgo_card4 != mtgo_card2);
  }

  SECTION("Card Move semantics")
  {
    // Test move constructors and move assignment

    mtgo::Card mtgo_card = mtgo::Card("1", "1", "name", "set", "rarity", true, 1.0, 2.0);
    mtgo::Card mtgo_card2 = mtgo::Card("1", "1", "name", "set", "rarity", true, 1.0, 2.0);

    // Move constructor
    mtgo::Card mtgo_card3(std::move(mtgo_card));
    CHECK(mtgo_card3 == mtgo_card2);
    // Check that mtgo_card is now invalid (commented out as it triggered warning in CI)
    // CHECK(mtgo_card.id_ == "");// Access of moved value

    // Move assignment
    auto mtgo_card_tmp = mtgo::Card("2", "1", "name", "set", "rarity", true, 1.0, 2.0);
    mtgo_card3 = std::move(mtgo_card_tmp);
    CHECK(mtgo_card3 != mtgo_card2);// ID should differ
    // Check that mtgo_card_tmp is now invalid (commented out as it triggered warning in CI)
    // CHECK(mtgo_card_tmp.id_ == ""); // Access of moved value (compiler warning)
  }
}

TEST_CASE("Command struct")
{
  // Command with no aliases
  constexpr clap::Command cmd{ "my-cmd", false };
  CHECK(cmd.name_ == "my-cmd");
  CHECK(cmd.flag_ == false);
  CHECK(cmd.aliases_.has_value() == false);

  // with alias
  constexpr clap::Command cmd_w_alias{ "my-cmd-with-alias", false, "my-cmd-alias" };
  CHECK(cmd_w_alias.name_ == "my-cmd-with-alias");
  CHECK(cmd_w_alias.flag_ == false);
  CHECK(cmd_w_alias.aliases_.has_value() == true);

  // With multiple aliases
  constexpr clap::Command cmd_w_aliases{ "my-cmd-with-aliases", true, "my-cmd-alias0", "my-cmd-alias1" };
  CHECK(cmd_w_aliases.name_ == "my-cmd-with-aliases");
  CHECK(cmd_w_aliases.flag_ == true);
  REQUIRE(cmd_w_aliases.aliases_.has_value() == true);
  CHECK(cmd_w_aliases.aliases_.value().at(0) == "my-cmd-alias0");
  CHECK(cmd_w_aliases.aliases_.value().at(1) == "my-cmd-alias1");

  // They can fit in same cmd array
  constexpr std::array<clap::Command, 3> cmd_arr = { cmd, cmd_w_alias, cmd_w_aliases };
  REQUIRE(cmd_arr.at(0).name_ == cmd.name_);
  CHECK(cmd.flag_ == false);
  CHECK(cmd.aliases_ == std::nullopt);

  REQUIRE(cmd_arr.at(2).name_ == "my-cmd-with-aliases");
  REQUIRE(cmd_arr.at(2).flag_ == true);
  REQUIRE(cmd_arr.at(2).aliases_.has_value() == true);
  CHECK(cmd_arr.at(2).aliases_.value().at(0).has_value() == true);
  CHECK(cmd_arr.at(2).aliases_.value().at(0).value() == "my-cmd-alias0");
  CHECK(cmd_arr.at(2).aliases_.value().at(1).value() == "my-cmd-alias1");
  CHECK(cmd_arr.at(2).aliases_.value().at(2).has_value() == false);
  // Compile-time error: static_assert failed: Too many aliases in initialization of struct Command
  // constexpr clap::Command cmd_w_too_many_aliases{
  //   "my-cmd-with-aliases", false, "my-cmd-alias0", "my-cmd-alias1", "my-cmd-alias2", "my-cmd-alias-one-too-many"
  // };

  constexpr clap::CommandArray<3> my_cmd_arr{ cmd, cmd_w_alias, cmd_w_aliases };
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

  constexpr int s = opt_arr.size();
  CHECK(s == 2);

  // clap::new_clap::Clap<1, 0>(
}

TEST_CASE("New and improved CLAP")
{

  constexpr clap::Option my_static_opt{ "--my-option", true, "--my-option-alis", "-m" };
  constexpr clap::OptionArray<1> my_static_opt_arr{ my_static_opt };
  constexpr clap::Command my_static_cmd{ "mycmd", true };
  constexpr clap::CommandArray<1> my_static_cmd_arr{ my_static_cmd };
  constexpr clap::new_clap::Clap new_static_clap{ my_static_opt_arr, my_static_cmd_arr };

  constexpr std::size_t cmd_count = new_static_clap.command_count();
  CHECK(cmd_count == 1);


  constexpr auto opt_count = new_static_clap.option_count();
  auto non_const_opt_count = new_static_clap.option_count();
  CHECK(opt_count == 1);
  CHECK(non_const_opt_count == 1);

  new_static_clap.PrintOptions();
}

// NOLINTEND