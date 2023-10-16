#pragma once

#include <fmt/core.h>

#include <algorithm>
#include <array>
#include <optional>
#include <string_view>


namespace clap {


// Struct for defining commands
struct Command
{
  std::string_view name_;
  bool is_flag_;

  template<std::convertible_to<std::string_view> T_Name,
    std::convertible_to<std::optional<std::string_view>>... T_Alias>
  [[nodiscard]] constexpr explicit Command(T_Name name, bool is_flag) noexcept : name_{ name }, is_flag_{ is_flag }
  {}
};

// Helper wrapper for a Command array
template<size_t N_cmds> struct CommandArray
{
  using T_cmd = clap::Command;

  std::array<T_cmd, N_cmds> cmds_;
  template<class... T> [[nodiscard]] constexpr explicit CommandArray(T... cmds) noexcept : cmds_{ cmds... } {}

  [[nodiscard]] constexpr auto size() const noexcept { return cmds_.size(); }

  void print() const noexcept
  {
    for (const T_cmd &cmd : cmds_) { fmt::print("{}\n", cmd.name_); }
  }

  [[nodiscard]] auto find(std::string_view cmd_name) const noexcept -> std::optional<T_cmd>
  {
    auto match_command_name = [&](const T_cmd &cmd) { return cmd.name_ == cmd_name; };


    if (auto iter = std::find_if(cmds_.begin(), cmds_.end(), match_command_name); iter != cmds_.end()) {
      return *iter;
    } else {
      return std::nullopt;
    }
  }
};


}// namespace clap