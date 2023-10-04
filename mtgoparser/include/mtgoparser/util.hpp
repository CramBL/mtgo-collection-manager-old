#pragma once
#include <charconv>
#include <optional>
#include <string_view>


namespace util {

template<typename SA, typename SB>
requires std::convertible_to<SA, std::string_view> && std::convertible_to<SB, std::string_view>
[[nodiscard]] inline constexpr auto is_sv_same(SA a, SB b) -> bool
{
  return static_cast<std::string_view>(a) == static_cast<std::string_view>(b);
}

template<typename SA, typename... Ss>
requires std::convertible_to<SA, std::string_view> &&(std::convertible_to<Ss, std::string_view> || ...)
  [[nodiscard]] inline constexpr auto is_sv_any_of(SA a, Ss... bs) -> bool
{
  return (is_sv_same(a, bs) || ...);
}


// No bounds check! Make sure it fits within `T_uint`.
template<typename T_uint> [[nodiscard]] inline auto sv_to_uint(std::string_view sv) -> std::optional<T_uint>
{
  T_uint value{};

  if (std::from_chars(sv.data(), sv.data() + sv.size(), value).ec == std::errc{}) {
    return value;
  } else {
    return std::nullopt;
  }
}
}// namespace util