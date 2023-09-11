#include <catch2/catch_test_macros.hpp>
#include <mtgoparser/clap.hpp>

constinit auto static_clap = clap::Clap<1>("--version");

TEST_CASE("Test misc. stuff")
{

  char argv0[] = "mtgo_preprocessor\0";
  char argv1[] = "--version\0";

  char *argv[] = { argv0, argv1 };
  int argc = 2;

  SECTION("Dynamically initialized - Show version")
  {
    auto clap = clap::Clap<1>("--version");
    fmt::print("Options are:\n");
    clap.PrintOptions();

    clap.Parse(argc, argv);
    fmt::print("Arguments are:\n");
    clap.PrintArgs();
  }

  SECTION("Static initialized - show version")
  {
    fmt::print("Parsing arguments with constinit Clap\n");
    static_clap.Parse(argc, argv);
    fmt::print("Arguments are:\n");
    static_clap.PrintArgs();
  }

  SECTION("Echo")
  {
    auto clap_2opts = clap::Clap<2>("--version", "--echo");
    clap_2opts.PrintOptions();
  }
}
