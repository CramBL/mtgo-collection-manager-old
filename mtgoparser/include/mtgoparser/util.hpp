#pragma once
#include <charconv>
#include <optional>
#include <string_view>


#include <boost/implicit_cast.hpp>

namespace util {

/**
 * @brief Check if two string-like values are the matching.
 *
 * A value is string-like if it is implicitly convertible to `std::string_view`.
 *
 * @tparam SA The type of the first string-like value
 * @tparam SB The type of the second string-like value
 * @param a_sv The first string-like value
 * @param b_sv The second string-like value
 * @return `true` if the string-like values match.
 * @return `false` if the string-like values do not match.
 *
 * @note The string-like values are converted to `std::string_view` before the comparison.
 */
template<typename SA, typename SB>
  requires std::convertible_to<SA, std::string_view> && std::convertible_to<SB, std::string_view>
[[nodiscard]] inline constexpr auto is_sv_same(SA a_sv, SB b_sv) -> bool
{
  return boost::implicit_cast<std::string_view>(a_sv) == boost::implicit_cast<std::string_view>(b_sv);
}

/**
 * @brief Check if a string-like value is the same as any of the other string-like values.
 *
 * A value is string-like if it is implicitly convertible to `std::string_view`.
 *
 * @tparam SA The type of the string-like value to compare against.
 * @tparam Ss The types of the other string-like values.
 *
 * @param a_sv The string-like value to compare against.
 * @param bs_svs The other string-like values.
 *
 * @return `true` if the string-like value is the same as any of the other string-like values.
 * @return `false` if the string-like value is not the same as any of the other string-like values.
 *
 * @note The string-like values are converted to `std::string_view` before the comparison.
 */
template<typename SA, typename... Ss>
  requires std::convertible_to<SA, std::string_view> && (std::convertible_to<Ss, std::string_view> || ...)
[[nodiscard]] inline constexpr auto is_sv_any_of(SA a_sv, Ss... bs_svs) -> bool
{
  return (is_sv_same(a_sv, bs_svs) || ...);
}


/**
 * @brief Convert a `string_view` to an unsigned integer.
 *
 * SAFETY: No bounds check is performed! Make sure the number in the string_view fits within `T_uint`
 *
 * @tparam T_uint The type of the unsigned integer.
 * @param sv The `string_view` to convert.
 *
 * @return std::optional<T_uint> The converted unsigned integer. `std::nullopt` if the conversion fails.
 *
 * @note The conversion fails if the `string_view` contains non-digit characters.
 *
 */
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