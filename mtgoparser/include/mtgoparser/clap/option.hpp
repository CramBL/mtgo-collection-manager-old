#pragma once

#include <cstdint>
#include <fmt/core.h>

#include <algorithm>
#include <array>
#include <optional>
#include <string_view>


namespace clap {

// More than 3 aliases is just too much
static inline constexpr size_t MAX_ALIAS_COUNT = 3;

enum class Opt : uint8_t { Flag, NeedValue };


// Struct for defining options
struct Option
{
  using T_opt_sv = std::optional<std::string_view>;
  using T_alias_array = std::array<T_opt_sv, clap::MAX_ALIAS_COUNT>;
  using T_alias_array_opt = std::optional<T_alias_array>;

  std::string_view name_;
  Opt opt_type_;
  // Array should be optional, will then be std::nullopt instead of empty array if there's no aliases
  T_alias_array_opt aliases_;


  template<std::convertible_to<std::string_view> T_Name, std::convertible_to<T_opt_sv>... T_Alias>
  [[nodiscard]] constexpr explicit Option(T_Name name, Opt opt_type, T_Alias... aliases) noexcept
    : name_{ name }, opt_type_{ opt_type }, aliases_{ T_alias_array{ T_opt_sv{ aliases }... } }
  {
    // Just to improve compiler error
    static_assert(
      sizeof...(aliases) <= clap::MAX_ALIAS_COUNT, "Too many aliases provided in initialization of struct `Option`");
  }

  [[nodiscard]] constexpr bool has_alias() const
  {
    return aliases_.has_value() && aliases_.value().front().has_value();
  }
};


// Helper wrapper for an Option array
template<size_t N_opts> struct OptionArray
{
  using T_opt = clap::Option;

  std::array<T_opt, N_opts> opts_;
  template<class... T> [[nodiscard]] constexpr explicit OptionArray(T... opts) noexcept : opts_{ opts... } {}

  [[nodiscard]] constexpr auto size() const noexcept { return opts_.size(); }

  void print() const noexcept
  {
    for (const T_opt &opt : opts_) { fmt::print("{}\n", opt.name_); }
  }

  [[nodiscard]] auto find(std::string_view opt_name) const noexcept -> std::optional<T_opt>
  {
    auto match_option_name = [&](const T_opt &opt) {
      return opt.name_ == opt_name
             || (opt.has_alias()
                 && std::any_of(opt.aliases_.value().begin(), opt.aliases_.value().end(), [&](const auto &name) {
                      return name.has_value() && name.value() == opt_name;
                    }));
    };


    if (auto iter = std::find_if(opts_.begin(), opts_.end(), match_option_name); iter != opts_.end()) [[likely]] {
      return *iter;
    } else [[unlikely]] {
      return std::nullopt;
    }
  }
};


}// namespace clap