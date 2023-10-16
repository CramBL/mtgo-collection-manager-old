#pragma once

#include "mtgoparser/clap/command.hpp"
#include "mtgoparser/clap/option.hpp"
#include "mtgoparser/clap/parser.hpp"

// Command-Line Argument Parsing (CLAP) utility
namespace clap {

// Define options
template<class... Opts> constexpr auto def_options(Opts... opts) noexcept -> decltype(auto)
{
  return clap::OptionArray<sizeof...(opts)>{ opts... };
}

// Define commands
template<class... Cs> constexpr auto def_cmds(Cs... cmds) noexcept -> decltype(auto)
{
  return clap::CommandArray<sizeof...(cmds)>{ cmds... };
}

// instantiate the CLAP
template<size_t N, size_t M>
[[nodiscard]] constexpr auto init_clap(clap::OptionArray<N> options_arr, clap::CommandArray<M> cmd_arr) noexcept
  -> clap::Clap<N, M>
{
  return clap::Clap<N, M>{ options_arr, cmd_arr };
}

}// namespace clap
