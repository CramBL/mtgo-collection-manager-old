#pragma once

#include "mtgoparser/clap/command.hpp"
#include "mtgoparser/clap/option.hpp"
#include "mtgoparser/clap/util.hpp"

#include <fmt/color.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <optional>
#include <string_view>
#include <utility>


namespace clap {


// The command-line argument parser class
template<size_t N_opts, size_t N_cmds> class Clap
{
  // User-defined options and commands
  std::optional<clap::OptionArray<N_opts>> options_;
  std::optional<clap::CommandArray<N_cmds>> commands_;

  // Options/command set from the command-line (generated from parsing the command-line arguments)
  std::optional<std::vector<std::pair<clap::Option, std::optional<std::string_view>>>> set_options_;
  std::optional<clap::Command> set_cmd_;// only single command allowed (TODO: support subcommands)
  bool is_clap_parsed = false;


public:
  [[nodiscard]] constexpr explicit Clap(clap::OptionArray<N_opts> opts_arr,
    clap::CommandArray<N_cmds> cmds_arr) noexcept
    : options_{ opts_arr }, commands_{ cmds_arr }, set_options_{ std::nullopt }, set_cmd_{ std::nullopt }
  {}

  [[nodiscard]] constexpr explicit Clap(std::optional<clap::OptionArray<N_opts>> opts_arr = std::nullopt,
    std::optional<clap::CommandArray<N_cmds>> cmds_arr = std::nullopt) noexcept
    : options_{ opts_arr }, commands_{ cmds_arr }, set_options_{ std::nullopt }, set_cmd_{ std::nullopt }
  {}

  [[nodiscard]] constexpr std::size_t option_count() const
  {
    if constexpr (N_opts == 0) {
      return 0;
    } else {
      return options_.value().size();
    }
  }

  [[nodiscard]] constexpr std::size_t command_count() const
  {
    if constexpr (N_cmds == 0) {
      return 0;
    } else {
      return commands_.value().size();
    }
  }

  void PrintOptions() const
  {
    if constexpr (N_opts == 0) {
      fmt::print("No options defined\n");
    } else {
      options_.value().print();
    }
  }

  void PrintCommands() const
  {
    if constexpr (N_cmds == 0) {
      fmt::print("No commands defined\n");
    } else {
      commands_.value().print();
    }
  }

  // Print help text when invoked with an option equivelant to "-h" or "help" in many command-line apps
  //
  // For long help (usually "--help") use PrintLongHelp
  void PrintShortHelp() const
  {
    spdlog::warn("TODO: Add description fields to Options and Commands");
    spdlog::warn("TODO: PrintAbout");
    spdlog::warn("TODO: PrintUsage");

    // Commands section
    fmt::print(fmt::emphasis::bold | fmt::emphasis::underline | fg(fmt::color::light_cyan), "Commands:\n");
    for (const clap::Command &cmd : this->commands_.value().cmds_) {
      fmt::print(fg(fmt::color::white), "  {}\n", cmd.name_);
    }
    fmt::print("\n");

    // Options section
    fmt::print(fmt::emphasis::bold | fmt::emphasis::underline | fg(fmt::color::light_yellow), "Options:\n");
    for (const clap::Option &opt : this->options_.value().opts_) {
      fmt::print(fg(fmt::color::white), "  {}\n", opt.name_);
    }
    fmt::print("\n");
  }

  // TODO: Long help

  // Returns the number of arguments that failed validation (check that it's 0 to not run over errors)
  [[nodiscard]] auto Parse(const std::vector<std::string_view> &args) noexcept -> size_t
  {
    if (this->is_clap_parsed) {
      return 1;
    } else {
      this->is_clap_parsed = true;
    }

    return this->parse_args(args);
  }


  void PrintArgs() const
  {
    if (this->set_cmd_.has_value()) {
      fmt::println("Set command: {}", this->set_cmd_.value().name_);
    } else {
      fmt::println("No command set");
    }
    if (this->set_options_.has_value()) {
      fmt::println("{} options set:\n", this->set_options_.value().size());
      for (const auto &opt : this->set_options_.value()) { fmt::print("\t{}\n", opt.first.name_); }
    } else {
      fmt::println("No options set");
    }
  }

  // Lookup if a flag is set by an Option instant, return if it's set or not
  [[nodiscard]] constexpr auto FlagSet(const clap::Option &opt_inst) const -> bool { return FlagSet(opt_inst.name_); }

  // Lookup if a flag is set by name, return if it's set or not
  [[nodiscard]] constexpr auto FlagSet(std::string_view flag_name) const -> bool
  {
    if constexpr (N_opts == 0) {
      return false;
    } else if (!this->set_options_.has_value()) {
      return false;
    } else {
      auto res = std::find_if(
        this->set_options_.value().begin(), this->set_options_.value().end(), [flag_name](const auto &opt) {
          return opt.first.name_ == flag_name
                 || (opt.first.has_alias()
                     && std::any_of(opt.first.aliases_.value().begin(),
                       opt.first.aliases_.value().end(),
                       [flag_name](const auto &name) { return name.has_value() && name.value() == flag_name; }));
        });

      return static_cast<bool>(res != this->set_options_.value().end());
    }
  }


  // Lookup option by supplying a partial or fully equivelant `Option` struct, and if it is set, get the value.
  [[nodiscard]] auto OptionValue(const clap::Option &opt_inst) const -> std::optional<std::string_view>
  {
    return OptionValue(opt_inst.name_);// :)
  }

  // Lookup option by name (or alias) and if it is set, get the value.
  [[nodiscard]] auto OptionValue(std::string_view opt_name) const -> std::optional<std::string_view>
  {
    if constexpr (N_opts == 0) {
      // If instantiated with 0 options
      return std::nullopt;
    } else if (!this->set_options_.has_value()) {
      // If no set options
      return std::nullopt;
    } else {
      // If set options, look for match

      auto res =
        std::find_if(this->set_options_.value().begin(), this->set_options_.value().end(), [opt_name](const auto &opt) {
          return opt.first.name_ == opt_name
                 || (opt.first.has_alias()
                     && std::any_of(opt.first.aliases_.value().begin(),
                       opt.first.aliases_.value().end(),
                       [opt_name](const auto &name) { return name.has_value() && name.value() == opt_name; }));
        });

      if (res != this->set_options_.value().end()) {
        if (!(*res).second.has_value()) {
          spdlog::warn("No value found for option: {}", (*res).first.name_);
          return std::nullopt;
        } else {
          return (*res).second.value();
        }
      } else {
        return std::nullopt;
      }
    }
  }

  // Lookup command by supplying a partial or fully equivelant `Command` struct, and return if it is set.
  [[nodiscard]] constexpr auto CmdSet(const clap::Command &cmd_inst) const -> bool { return CmdSet(cmd_inst.name_); }

  // Lookup command by name, and return if it is set or not
  [[nodiscard]] constexpr auto CmdSet(std::string_view cmd_name) const -> bool
  {
    if constexpr (N_cmds == 0) {
      return false;
    } else {
      return (this->set_cmd_.has_value() && this->set_cmd_.value().name_ == cmd_name);
    }
  }

  [[nodiscard]] auto isClapParsed() const noexcept -> bool { return this->is_clap_parsed; }

  // Helpers
private:
  void store_option_value(clap::Option opt, std::optional<std::string_view> opt_val)
  {
    if (!this->set_options_.has_value()) {
      this->set_options_ =
        std::vector<std::pair<clap::Option, std::optional<std::string_view>>>{ std::make_pair(opt, opt_val) };
    } else {
      this->set_options_.value().emplace_back(std::make_pair(opt, opt_val));
    }
  }

  [[nodiscard]] auto find_command(std::string_view cmd) -> size_t
  {
    if (auto found_cmd = this->commands_.value().find(cmd)) {
      if (!this->set_cmd_.has_value()) {
        this->set_cmd_ = std::move(found_cmd);
      } else [[unlikely]] {
        spdlog::error(
          "Attempted setting command: '{}' when a command was already set: '{}'. Only one command is allowed",
          cmd,
          this->set_cmd_.value().name_);
        return 1;
      }
    }
    return 0;
  }


  [[nodiscard]] auto parse_args(const std::vector<std::string_view> &args) noexcept -> size_t
  {

    size_t errors = 0;

    for (auto it = args.cbegin(), end = args.cend(); it != end; ++it) {
      if ((*it)[0] == '-') {
        // Find in option array
        if constexpr (N_opts == 0) {
          ++errors;
          spdlog::error("Got option '{}' but no options have been defined", *it);
        } else {
          if (std::optional<clap::Option> found_opt = this->options_.value().find(*it)) {
            // Check if it is not a flag, then the next argument should be the attached value
            std::optional<std::string_view> opt_value = std::nullopt;
            if (found_opt.value().opt_type_ == Opt::NeedValue) {
              // Then check for the value in the arguments
              if ((it + 1 == end) || (*(it + 1)).starts_with("-")) [[unlikely]] {
                ++errors;
                spdlog::error("Option {} passed with missing value", *it);
              } else {
                opt_value = *(it + 1);
                // Increment as we already validated the option value and we don't want to parse it as an option in
                // the next iteration
                ++it;
              }
            }
            this->store_option_value(found_opt.value(), opt_value);
          } else [[unlikely]] {
            // Provided option not found
            ++errors;
            spdlog::error("Unknown option '{}'", *it);
          }
        }

      } else {
        // Find in command array
        if constexpr (N_cmds == 0) {
          ++errors;
          spdlog::error("Got command '{}' but no commands have been defined", *it);
        } else {
          errors += this->find_command(*it);
        }
      }
    }

    return errors;
  }
};


}// namespace clap