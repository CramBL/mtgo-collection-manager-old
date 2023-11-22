#pragma once
#include <charconv>
#include <optional>
#include <string_view>

#include <fmt/core.h>

#include <boost/outcome.hpp>
#include <boost/outcome/result.hpp>

#include <boost/implicit_cast.hpp>

namespace util {

namespace outcome = BOOST_OUTCOME_V2_NAMESPACE;
using ErrorStr = std::string;

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
requires std::convertible_to<SA, std::string_view> &&(std::convertible_to<Ss, std::string_view> || ...)
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
 * @return `outcome::result<T_uint, ErrorStr>` containing the converted value if the conversion succeeds.
 * @return `outcome::failure(ErrorStr)` if the conversion fails.
 *
 * @note The conversion fails if the `string_view` contains non-digit characters.
 *
 */
template<typename T_uint> [[nodiscard]] inline auto sv_to_uint(std::string_view sv) -> outcome::result<T_uint, ErrorStr>
{
  T_uint value{};

  if (std::from_chars(sv.data(), sv.data() + sv.size(), value).ec == std::errc{}) [[likely]] {
    return outcome::success(value);
  } else [[unlikely]] {
    return outcome::failure(fmt::format("Failed to convert string_view `{}` to uint", sv));
  }
}

}// namespace util

namespace util::mp {

/**
 * @brief Returns true if a type is the same as any of the other types.
 *
 * @tparam T The type to compare against.
 * @tparam CompareToTypes The types to compare with.
 */
template<typename T, typename... CompareToTypes>
inline constexpr bool is_t_any = std::disjunction_v<std::is_same<T, CompareToTypes>...>;


/**
 * @brief Returns true if a type is the same as all of the other types.
 *
 * @tparam T The type to compare against.
 * @tparam CompareToTypes The types to compare with.
 */
template<typename T, typename... CompareToTypes>
inline constexpr bool is_t_same = std::conjunction_v<std::is_same<T, CompareToTypes>...>;

}// namespace util::mp

namespace util::optimization {

/**
 * @brief Branchless if statement that returns either `true_val` or `false_val` depending on
 * `cond`.
 *
 * @warning Profile before committing! This function is SLOWER if any of the following are true: The branch is rarely
 * mispredicted, the values are expensive to evaluate, or the compiler already does a branchless optimization (always
 * preferred).
 *
 * @note The tradeoff is that both `true_val` and `false_val` are evaluated, so if they are
 * expensive to evaluate then this function is not useful. This function is useful when the values are cheap to evaluate
 * and the branch is likely to be mispredicted (i.e. there's no clear pattern to the values of `cond`).
 *
 * @tparam T Type of the return value.
 * @param cond The condition to check.
 * @param true_val The value to return if `cond` is true.
 * @param false_val The value to return if `cond` is false.
 * @return T Either `true_val` or `false_val` depending on `cond`.
 */
template<typename T> [[nodiscard]] inline constexpr auto branchless_if(bool cond, T false_val, T true_val) -> T
{
  std::array ret_vals = { false_val, true_val };
  return ret_vals[boost::implicit_cast<uint8_t>(cond)];
}


}// namespace util::optimization