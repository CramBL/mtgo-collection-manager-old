#pragma once
#include <functional>
#include <mtgoparser/clap.hpp>

namespace config {

namespace option {
  constexpr clap::Option mtgoupdater_json_out{ "--collection-json-out", true };
  constexpr clap::Option help{ "-h", true, "--help" };
  constexpr clap::Option debug{ "-d", true, "--debug" };
  constexpr clap::Option update{ "-u", true, "--update", "--update-all" };
  constexpr clap::Option scryfall_path{ "--scryfall-path", false };
  constexpr clap::Option fulltradelist_path{ "--full-trade-list", false };
  constexpr clap::Option card_defs_path{ "--card-definitions", false };
  constexpr clap::Option price_hist_path{ "--price-history", false };
  constexpr clap::Option app_data_dir{ "--appdata-dir", false };
  constexpr clap::Option echo{ "--echo", true };


  constexpr clap::OptionArray opt_array = clap::def_options(clap::Option("--version", true, "-V"),
    help,
    debug,
    update,
    scryfall_path,
    fulltradelist_path,
    card_defs_path,
    price_hist_path,
    echo,
    mtgoupdater_json_out,
    app_data_dir);
}// namespace option

namespace commands {
  constexpr clap::Command run{ "run", true };
}

class Config
{
public:
  static auto get() -> decltype(auto)
  {
    static constinit auto config = clap::init_clap(option::opt_array, clap::def_cmds(commands::run));
    return &config;
  }
};

}// namespace config