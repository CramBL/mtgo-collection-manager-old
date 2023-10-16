#pragma once

#include <spdlog/spdlog.h>

#include <algorithm>
#include <optional>
#include <string_view>
#include <type_traits>
#include <vector>


namespace clap::util {

// Helper function to check if a value equals any element in a parameter pack
template<typename T, typename... Args>
[[nodiscard]] inline constexpr auto equals_any(const T &value, Args... args) -> bool
{
  return ((value == args) || ...);
}

// Type trait to check if a type is convertible to std::string_view
template<typename T> struct is_convertible_to_string_view
{
  static constexpr bool value = std::is_convertible_v<T, std::string_view>;
};

// Helper function to check if all types in a parameter pack are convertible to std::string_view
template<typename... Args> [[nodiscard]] inline constexpr auto all_convertible_to_string_view() -> bool
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
    args.cbegin(), args.cend(), [&](const std::string_view &arg) { return util::equals_any(arg, option_names...); });
}

// Returns the argument to an option if the option or any of its aliases exists and it has an argument
template<typename... Options>
[[nodiscard]] auto has_option_arg(const std::vector<std::string_view> &args, Options... option_names)
  -> std::optional<std::string_view>
{
  static_assert(all_convertible_to_string_view<Options...>(), "Options must be convertible to std::string_view");


  for (auto it = args.cbegin(), end = args.cend(); it != end; ++it) {
    if (util::equals_any(*it, option_names...)) {
      if (it + 1 != end) {
        return *(it + 1);
      } else {
        spdlog::error("Option {} was specified but no argument was given", *it);
      }
    }
  }

  return std::nullopt;
}


}// namespace clap::util