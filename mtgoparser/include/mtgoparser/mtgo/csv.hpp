#pragma once

#include <boost/cstdfloat.hpp>
#include <boost/implicit_cast.hpp>
#include <boost/lexical_cast.hpp>

#include <optional>
#include <sstream>
#include <string_view>
#include <utility>
#include <vector>

#ifdef __llvm__
#define LLVM_ASSUME(expr) __builtin_assume(expr)
#else
#define LLVM_ASSUME(expr) ((void)0)
#endif


// Function to split a string_view into a vector of sub-views based on a delimiter
[[nodiscard]] inline auto constexpr into_token_vec(std::string_view str, char delimiter)
  -> std::vector<std::string_view>
{
  std::vector<std::string_view> sub_views;
  std::size_t start = 0;
  std::size_t end = str.find(delimiter);

  while (end != std::string_view::npos) {
    sub_views.push_back(str.substr(start, end - start));
    start = end + 1;
    end = str.find(delimiter, start);
  }

  // Add the last token
  sub_views.push_back(str.substr(start));

  return sub_views;
}

using opt_float_t = std::optional<boost::float32_t>;

// Function to parse a string into two floats, handling the case where a hyphen signifies a missing value
[[nodiscard]] inline auto sv_to_floats(std::string_view str) -> std::pair<opt_float_t, opt_float_t>
{


  [[maybe_unused]] std::size_t size = str.size();
  LLVM_ASSUME(size < 32);


  constexpr char delimiter = ';';
  std::size_t delim_pos = str.find(delimiter);
  std::string_view gb_price_str = str.substr(0, delim_pos);
  std::string_view scryfall_opt_str = str.substr(delim_pos + 1);


  opt_float_t first = gb_price_str == "-" ? boost::implicit_cast<opt_float_t>(std::nullopt)
                                          : boost::lexical_cast<float32_t>(gb_price_str);
  opt_float_t second = scryfall_opt_str == "-" ? boost::implicit_cast<opt_float_t>(std::nullopt)
                                               : boost::lexical_cast<float32_t>(scryfall_opt_str);
  return std::make_pair(first, second);
}
