#pragma once
#include <functional>
#include <mtgoparser/clap.hpp>
#include <mtgoparser/clap/option.hpp>

namespace config {

using clap::Opt::NeedValue;

namespace option {
  constexpr clap::Option mtgoupdater_json_out{ "--collection-json-out", clap::Opt::Flag };
  constexpr clap::Option help{ "-h", clap::Opt::Flag, "--help" };
  constexpr clap::Option debug{ "-d", clap::Opt::Flag, "--debug" };
  constexpr clap::Option update{ "-u", clap::Opt::Flag, "--update", "--update-all" };
  constexpr clap::Option scryfall_path{ "--scryfall-path", clap::Opt::NeedValue };
  constexpr clap::Option fulltradelist_path{ "--full-trade-list", clap::Opt::NeedValue };
  constexpr clap::Option card_defs_path{ "--card-definitions", NeedValue };
  constexpr clap::Option price_hist_path{ "--price-history", NeedValue };
  constexpr clap::Option app_data_dir{ "--appdata-dir", NeedValue };
  constexpr clap::Option echo{ "--echo", clap::Opt::Flag };


  constexpr clap::OptionArray opt_array = clap::def_options(clap::Option("--version", clap::Opt::Flag, "-V"),
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