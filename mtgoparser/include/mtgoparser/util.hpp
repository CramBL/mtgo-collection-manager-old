#pragma once
#include <charconv>
#include <optional>
#include <string_view>


namespace util {
// No bounds check! Make sure it fits within `T_uint`.
template<typename T_uint> std::optional<T_uint> sv_to_uint(std::string_view sv)
{
  T_uint value{};

  if (std::from_chars(sv.data(), sv.data() + sv.size(), value).ec == std::errc{}) {
    return value;
  } else {
    return std::nullopt;
  }
}
}// namespace util