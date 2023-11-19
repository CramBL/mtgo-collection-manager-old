#pragma once

#include <boost/implicit_cast.hpp>

#include <optional>
#include <string>
#include <utility>
#include <vector>

#ifdef __llvm__
#define LLVM_ASSUME(expr) __builtin_assume(expr)
#else
#define LLVM_ASSUME(expr) ((void)0)
#endif

namespace mtgo::csv {


// Function to split a string_view into a vector of sub-views based on a delimiter
[[nodiscard]] inline auto into_substr_vec(const std::string &str, char delimiter) -> std::vector<std::string>
{
  std::vector<std::string> sub_strs;
  std::size_t start = 0;
  std::size_t end = str.find(delimiter);

  while (end != std::string::npos) {
    sub_strs.push_back(str.substr(start, end - start));
    start = end + 1;
    end = str.find(delimiter, start);
  }

  // Add the last token
  sub_strs.push_back(str.substr(start));

  return sub_strs;
}

using opt_float_t = std::optional<float>;

// Function to parse a string into two floats, handling the case where a hyphen signifies a missing value
[[nodiscard]] inline auto str_to_floats(const std::string &str) -> std::pair<opt_float_t, opt_float_t>
{
  constexpr char delimiter = ';';
  const std::size_t delim_pos = str.find(delimiter);
  const std::string gb_price_str = str.substr(0, delim_pos);

  {// Removes the out of bounds checks and exception instructions from the assembly (https://godbolt.org/z/GP5jfPz57)
    [[maybe_unused]] const std::size_t size = str.size();
    LLVM_ASSUME(delim_pos < size);
  }
  const std::string scryfall_opt_str = str.substr(delim_pos + 1);


  opt_float_t gb_price =
    gb_price_str == "-" ? boost::implicit_cast<opt_float_t>(std::nullopt) : std::stof(gb_price_str);
  opt_float_t scryfall_price =
    scryfall_opt_str == "-" ? boost::implicit_cast<opt_float_t>(std::nullopt) : std::stof(scryfall_opt_str);

  return std::make_pair(gb_price, scryfall_price);
}

}// namespace mtgo::csv