#pragma once

#include <boost/implicit_cast.hpp>

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

namespace mtgo::csv {


// Function to split a string_view into a vector of sub-views based on a delimiter
[[nodiscard]] inline auto constexpr into_substr_vec(const std::string &str, char delimiter) -> std::vector<std::string>
{
  std::vector<std::string> sub_strs;
  std::size_t start = 0;
  std::size_t end = str.find(delimiter);

  while (end != std::string_view::npos) {
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
  [[maybe_unused]] std::size_t size = str.size();
  LLVM_ASSUME(size < 32);

  std::istringstream ss(str);

  opt_float_t first_val{};
  opt_float_t second_val{};

  if (ss.peek() == '-') {
    ss.ignore();
  } else {
    ss >> *first_val;
  }
  ss.ignore();
  if (ss.peek() == '-') {
    ss.ignore();
  } else {
    ss >> *second_val;
  }

  return std::make_pair(first_val, second_val);
}

}// namespace mtgo::csv