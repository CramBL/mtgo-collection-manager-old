#pragma once
#include <algorithm>
#include <array>
#include <concepts>
#include <optional>
#include <spdlog/spdlog.h>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

// Command-Line Argument Parsing (CLAP) utility
namespace clap {
namespace {// Utility used by the Clap class

  // Helper function to check if a value equals any element in a parameter pack
  template<typename T, typename... Args> constexpr auto equals_any(const T &value, Args... args) -> bool
  {
    return ((value == args) || ...);
  }

  // Type trait to check if a type is convertible to std::string_view
  template<typename T> struct is_convertible_to_string_view
  {
    static constexpr bool value = std::is_convertible_v<T, std::string_view>;
  };

  // Helper function to check if all types in a parameter pack are convertible to std::string_view
  template<typename... Args> constexpr bool all_convertible_to_string_view()
  {
    return (is_convertible_to_string_view<Args>::value && ...);
  }

  // Check if an option or any of its aliases are set
  template<typename... Options>
  [[nodiscard]] auto has_option(const std::vector<std::string_view> &args, Options... option_names) -> bool
  {
    static_assert(all_convertible_to_string_view<Options...>(), "Options must be convertible to std::string_view");

    // Cannot use std::ranges because apple clang still does not support it...
    return std::any_of(
      args.cbegin(), args.cend(), [&](const std::string_view &arg) { return equals_any(arg, option_names...); });
  }

  // Returns the argument to an option if the option or any of its aliases exists and it has an argument
  template<typename... Options>
  [[nodiscard]] auto has_option_arg(const std::vector<std::string_view> &args, Options... option_names)
    -> std::optional<std::string_view>
  {
    static_assert(all_convertible_to_string_view<Options...>(), "Options must be convertible to std::string_view");


    for (auto it = args.cbegin(), end = args.cend(); it != end; ++it) {
      if (equals_any(*it, option_names...)) {
        if (it + 1 != end) {
          return *(it + 1);
        } else {
          spdlog::error("Option {} was specified but no argument was given", *it);
        }
      }
    }

    return std::nullopt;
  }

}// namespace


// More than 3 aliases is just too much
static inline constexpr size_t MAX_ALIAS_COUNT = 3;

// Struct for defining options
struct Option
{
  std::string_view name_;
  bool flag_;
  // Array should be optional, will then be std::nullopt instead of empty array if there's no aliases
  std::optional<std::array<std::optional<std::string_view>, clap::MAX_ALIAS_COUNT>> aliases_;

  template<std::convertible_to<std::string_view> T_Name,
    std::convertible_to<std::optional<std::string_view>>... T_Alias>
  [[nodiscard]] constexpr explicit Option(T_Name name, bool is_flag, T_Alias... aliases) noexcept
    : name_{ name }, flag_{ is_flag }
  {
    // Just to improve compiler error
    static_assert(
      sizeof...(aliases) <= clap::MAX_ALIAS_COUNT, "Too many aliases provided in initialization of struct `Option`");

    aliases_ = { aliases... };
  }

  constexpr bool has_alias() const { return aliases_.has_value() && aliases_.value().front().has_value(); }
};

// Struct for defining commands
struct Command
{
  std::string_view name_;
  bool flag_;

  template<std::convertible_to<std::string_view> T_Name,
    std::convertible_to<std::optional<std::string_view>>... T_Alias>
  [[nodiscard]] constexpr explicit Command(T_Name name, bool is_flag) : name_{ name }, flag_{ is_flag }
  {}
};

// Helper wrapper for a Command array
template<size_t N_cmds> struct CommandArray
{
  using T_cmd = clap::Command;

  std::array<T_cmd, N_cmds> cmds_;
  template<class... T> [[nodiscard]] constexpr explicit CommandArray(T... cmds) : cmds_{ cmds... } {}

  [[nodiscard]] constexpr auto size() const { return cmds_.size(); }

  void print() const
  {
    for (const T_cmd &cmd : cmds_) { fmt::print("{}\n", cmd.name_); }
  }

  [[nodiscard]] auto find(std::string_view cmd_name) const -> std::optional<T_cmd>
  {
    auto match_command_name = [&](const T_cmd &cmd) { return cmd.name_ == cmd_name; };


    if (auto it = std::find_if(cmds_.begin(), cmds_.end(), match_command_name); it != cmds_.end()) {
      return *it;
    } else {
      return std::nullopt;
    }
  }
};

// Helper wrapper for an Option array
template<size_t N_opts> struct OptionArray
{
  using T_opt = clap::Option;

  std::array<T_opt, N_opts> opts_;
  template<class... T> [[nodiscard]] constexpr explicit OptionArray(T... opts) : opts_{ opts... } {}

  [[nodiscard]] constexpr auto size() const { return opts_.size(); }

  void print() const
  {
    for (const T_opt &opt : opts_) { fmt::print("{}\n", opt.name_); }
  }

  [[nodiscard]] auto find(std::string_view opt_name) const -> std::optional<T_opt>
  {
    auto match_option_name = [&](const T_opt &opt) {
      return opt.name_ == opt_name
             || (opt.has_alias()
                 && std::any_of(opt.aliases_.value().begin(), opt.aliases_.value().end(), [&](const auto &a) {
                      return a.has_value() && a.value() == opt_name;
                    }));
    };


    if (auto it = std::find_if(opts_.begin(), opts_.end(), match_option_name); it != opts_.end()) {
      return *it;
    } else {
      return std::nullopt;
    }
  }
};

// The command-line argument parser class
template<size_t N_options> class Clap
{
  std::array<std::pair<std::string_view, bool>, N_options> _options;
  std::optional<std::vector<std::string_view>> _args;

  // Returns the number of arguments that failed validation
  [[nodiscard]] auto validate_args() noexcept -> size_t
  {
    size_t errors = 0;
    for (auto it = _args.value().cbegin(), end = _args.value().cend(); it != end; ++it) {

      auto is_defined = [it](std::pair<std::string_view, bool> opt_p) { return opt_p.first == *it; };

      if (auto opt_it = std::find_if(_options.cbegin(), _options.cend(), is_defined); opt_it != std::end(_options)) {
        // Check if it is an option that should have a value
        if ((*opt_it).second) {
          // Then check for the value in the arguments
          if ((it + 1 == end) || (*(it + 1)).starts_with("-")) {
            ++errors;
            spdlog::error("Option passed with missing value");
          } else {
            // Increment as we already validated the option value and we don't want to parse it as an option in the
            // next iteration
            ++it;
          }
        }
      } else {
        ++errors;
        spdlog::error("Unknown option: {}", *it);
      }
    }
    return errors;
  }


public:
  template<std::convertible_to<std::string_view>... Options>
  [[nodiscard]] constexpr explicit Clap(std::pair<Options, bool>... opts) noexcept
  {
    static_assert(sizeof...(Options) == N_options);

    _options = { opts... };
  }

  // Returns the number of arguments that failed validation (check that it's 0 to not run over errors)
  [[nodiscard]] auto Parse(int argc, char *argv[]) noexcept -> size_t
  {
    _args = std::vector<std::string_view>(argv + 1, argv + argc);
    return validate_args();
  }

  void PrintOptions() const
  {
    for (const auto &opt : _options) { fmt::print("{}\n", opt.first); }
  }

  void PrintArgs() const
  {
    if (_args.has_value()) {
      for (const auto &arg : _args.value()) { fmt::print("{}\n", arg); }
    } else {
      spdlog::warn("No arguments found - did you remember to parse them first?");
    }
  }

  template<std::convertible_to<std::string_view>... Flags> [[nodiscard]] auto FlagSet(Flags... flags) -> bool
  {
    if (!_args.has_value()) {
      spdlog::warn("Attempted to check if a CL flag was set before parsing CL arguments");
      return false;
    }
    return has_option(_args.value(), flags...);
  }

  template<std::convertible_to<std::string_view>... Options>
  [[nodiscard]] auto OptionValue(Options... opts) -> std::optional<std::string_view>
  {
    if (!_args.has_value()) {
      spdlog::warn("Attempted to retrieve an CL option value before parsing CL arguments");
      return std::nullopt;
    }
    return has_option_arg(_args.value(), opts...);
  }
};

namespace new_clap {

  // The command-line argument parser class
  template<size_t N_opts, size_t N_cmds> class Clap
  {
    // User-defined options and commands
    std::optional<clap::OptionArray<N_opts>> options_;
    std::optional<clap::CommandArray<N_cmds>> commands_;

    // Options/command set from the command-line (generated from parsing the command-line arguments)
    std::optional<std::vector<clap::Option>> set_options_;
    std::optional<clap::Command> set_cmd_;// only single command allowed (TODO: support subcommands)

  public:
    [[nodiscard]] constexpr explicit Clap(clap::OptionArray<N_opts> opts_arr,
      clap::CommandArray<N_cmds> cmds_arr) noexcept
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

    // Returns the number of arguments that failed validation (check that it's 0 to not run over errors)
    [[nodiscard]] auto Parse(int argc, char *argv[]) noexcept -> size_t
    {
      size_t errors = 0;

      auto tmp_args = std::vector<std::string_view>(argv + 1, argv + argc);

      for (const auto &arg : tmp_args) {
        // Option
        if (arg[0] == '-') {
          // Find in option array
          if constexpr (N_opts == 0) {
            ++errors;
            spdlog::error("Got option '{}' but no options have been defined", arg);
          } else {
            if (std::optional<clap::Option> found_opt = this->options_.value().find(arg)) {
              if (!this->set_options_.has_value()) {
                this->set_options_ = std::vector<clap::Option>{ std::move(found_opt.value()) };
              } else {
                this->set_options_.value().emplace_back(std::move(found_opt.value()));
              }
            }
          }
        } else {
          // Find in command array
          if constexpr (N_cmds == 0) {
            ++errors;
            spdlog::error("Got command '{}' but no commands have been defined", arg);
          } else {
            if (auto found_cmd = this->commands_.value().find(arg)) {
              if (!this->set_cmd_.has_value()) {
                this->set_cmd_ = std::move(found_cmd);
              } else {
                ++errors;
                spdlog::error(
                  "Attempted setting command: '{}' when a command was already set: '{}'. Only one command is allowed",
                  arg,
                  this->set_cmd_.value().name_);
              }
            }
          }
        }
      }
      // TODO: return validate_args();
      return errors;
    }


    void PrintArgs() const
    {
      if (this->set_cmd_.has_value()) {
        fmt::print("Set command: {}", this->set_cmd_.value().name_);
      } else {
        fmt::print("No command set\n");
      }
      if (this->set_options_.has_value()) {
        fmt::print("{} options set:\n", this->set_options_.value().size());
        for (const auto &opt : this->set_options_.value()) { fmt::print("\t{}\n", opt.name_); }
      } else {
        fmt::print("No options set\n");
      }
    }

    // template<std::convertible_to<std::string_view>... Flags> [[nodiscard]] auto FlagSet(Flags... flags) -> bool
    // {
    //   if (!args_.has_value()) {
    //     spdlog::warn("Attempted to check if a CL flag was set before parsing CL arguments");
    //     return false;
    //   }
    //   return has_option(args_.value(), flags...);
    // }
  };


}// namespace new_clap

}// namespace clap
