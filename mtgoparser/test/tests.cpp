// NOLINTBEGIN
#include <catch2/catch_test_macros.hpp>
#include <mtgoparser/clap.hpp>
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
// NOLINTEND