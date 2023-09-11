#include <array>
#include <concepts>
#include <optional>
#include <spdlog/spdlog.h>
#include <string_view>
#include <type_traits>
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
      args.begin(), args.end(), [&](const std::string_view &arg) { return equals_any(arg, option_names...); });
  }

  // Returns the argument to an option if the option or any of its aliases exists and it has an argument
  template<typename... Options>
  [[nodiscard]] auto has_option_arg(const std::vector<std::string_view> &args, Options... option_names)
    -> std::optional<std::string_view>
  {
    static_assert(all_convertible_to_string_view<Options...>(), "Options must be convertible to std::string_view");


    for (auto it = args.begin(), end = args.end(); it != end; ++it) {
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

template<size_t N_options> class Clap
{
  std::array<std::string_view, N_options> _options;
  std::optional<std::vector<std::string_view>> _args;

public:
  template<std::convertible_to<std::string_view>... Options> [[nodiscard]] constexpr explicit Clap(Options... opts)
  {
    static_assert(sizeof...(Options) == N_options);

    _options = { opts... };
  }

  void Parse(int argc, char *argv[]) { _args = std::vector<std::string_view>(argv + 1, argv + argc); }

  void PrintOptions() const
  {
    for (const auto &opt : _options) { fmt::print("{}\n", opt); }
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


}// namespace clap
