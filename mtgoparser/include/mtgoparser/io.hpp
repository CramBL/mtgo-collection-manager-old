#pragma once

// NOLINTBEGIN
#include <toml++/toml.hpp>
// NOLINTEND

#include <boost/implicit_cast.hpp>
#include <boost/outcome.hpp>
#include <boost/outcome/result.hpp>

#include <fmt/core.h>

#include <filesystem>
#include <format>
#include <fstream>
#include <ios>
#include <string>
#include <string_view>
#include <vector>

namespace io_util {

namespace outcome = BOOST_OUTCOME_V2_NAMESPACE;
namespace fs = std::filesystem;

// For failure cases, return a string describing the error
using ErrorStr = std::string;

[[nodiscard]] inline auto read_to_str_buf(const fs::path &fpath) -> std::string
{
  // Open the stream to 'lock' the file.
  std::ifstream file(fpath, std::ios::in | std::ios::binary);

  // Obtain the size of the file.
  const auto fsize = fs::file_size(fpath);

  // Create a buffer.
  std::string str_buffer(fsize, '\0');

  // Read the whole file into the buffer.
  file.read(str_buffer.data(), static_cast<std::streamsize>(fsize));

  return str_buffer;
}

[[nodiscard]] inline auto read_to_char_buf(const fs::path &fpath) -> std::vector<char>
{
  // Open the stream to 'lock' the file.
  std::ifstream file(fpath, std::ios::in | std::ios::binary);

  // Obtain the size of the file.
  const auto fsize = fs::file_size(fpath);

  // Instantiate and pre-allocate
  std::vector<char> char_buf{};
  char_buf.resize(fsize + 1);// +1 for null termination

  // Null terminate (guaranteed to not be overwritten by the following call to read())
  char_buf[fsize] = '\0';

  // Read into buffer
  file.read(char_buf.data(), static_cast<std::streamsize>(fsize));

  return char_buf;
}

/**
 * @brief Read a TOML file into a TOML table.
 *
 * @param str_fpath The path to the TOML file
 * @return decltype(auto) The TOML table
 */
[[nodiscard]] inline auto read_state_log(const std::string_view str_fpath) -> decltype(auto)
{
  return toml::parse_file(str_fpath);
}


/**
 * @brief Get the current time as a UTC ISO 8601 timestamp %Y-%m-%dT%H%M%SZ without sub-second precision.
 *
 * e.g. `2023-11-05T152700Z`
 *
 * @return `std::string` The timestamp
 */
[[nodiscard]] inline auto now_utc_iso8601_timestamp() -> std::string
{
  // Convert to a timestamp in UTC and %Y-%m-%dT%H:%M:%SZ which is ISO 8601
  // using formatter: https://en.cppreference.com/w/cpp/chrono/system_clock/formatter

  // Get the current time
  const auto now = std::chrono::system_clock::now();

  // Apple clang is behind on C++20 support, so use std::put_time instead of std::format
#if defined(__APPLE__) && defined(__llvm__) && __clang_major__ == 15
  auto now_time_t = std::chrono::system_clock::to_time_t(now);

  // Convert to ISO 8601 format
  std::ostringstream oss;
  oss << std::put_time(std::gmtime(&now_time_t), "%Y-%m-%dT%H%M%SZ");
  std::string now_utc_iso8601_timestamp = oss.str();
#else
  // std::vformat is the way to specify a runtime format string in C++20
  std::string tmp_iso8601_timestamp = std::vformat("{:%FT%TZ}", std::make_format_args(now));
  // It has sub-second precision, so remove the decimal point and everything after it, then add a 'Z' to indicate UTC
  std::string now_utc_iso8601_timestamp = tmp_iso8601_timestamp.substr(0, tmp_iso8601_timestamp.find('.')) + 'Z';
  // Erase-remove idiom to remove colons from timestamp as they are not allowed in Windows file names
  now_utc_iso8601_timestamp.erase(std::remove(now_utc_iso8601_timestamp.begin(), now_utc_iso8601_timestamp.end(), ':'),
    now_utc_iso8601_timestamp.end());
#endif

  return now_utc_iso8601_timestamp;
}


/**
 * @brief Save a string buffer to a file with a UTC ISO 8601 timestamp appended to the file name.
 *
 * Any directories in the path that do not exist will be created.
 * The timestamp is in the format %Y-%m-%dT%H%M%SZ without sub-second precision.
 *  e.g. `2023-11-05T152700Z`.
 * A file called `my_file.txt` will be saved as `my_file_2023-11-05T152700Z.txt`.
 *
 * @param buf The string buffer to save
 *
 * @param fpath The path to the file to save to
 *
 * @param ext The file extension to append to the file name
 *
 * @return On success: The path to the saved file
 * @return On failure: The error message
 */
[[nodiscard]] inline auto save_with_timestamp(const auto &buf, const fs::path &fpath, const auto &ext)
  -> outcome::result<fs::path, ErrorStr>
{
  try {
    // If the path is not to the current directory
    if (fpath.has_parent_path()) {
      // Create directories if they don't exist
      fs::create_directories(fpath.parent_path());
    }

    // Get the current time as a UTC ISO 8601 timestamp %Y-%m-%dT%H%M%SZ without sub-second precision
    std::string now_timestamp = io_util::now_utc_iso8601_timestamp();

    std::string final_fname = fmt::format("{}_{}.{}", fpath.stem().string(), now_timestamp, ext);

    fs::path fpath_with_time =
      fpath.has_parent_path() ? fpath.parent_path() / final_fname : boost::implicit_cast<fs::path>(final_fname);

    // Open the file
    std::ofstream file(fpath_with_time, std::ios::binary | std::ios::out);

    if (file.bad()) {
      return outcome::failure(fmt::format("Bad operation during opening of file: {}", fpath_with_time.string()));
    }

    if (file.is_open()) {
      // Write the buffer to the file
      file.write(buf.data(), static_cast<std::streamsize>(buf.size()));
      // Close the file
      file.close();
    } else {
      return outcome::failure(fmt::format("Expected file to be open: {}", fpath_with_time.string()));
    }


    // Return the file name
    return outcome::success(fpath_with_time);

  } catch (const std::exception &e) {
    return outcome::failure(fmt::format("Failed to save file from path: {}, err: {}", fpath.string(), e.what()));
  }
}

/**
 * @brief Check if two files are equal by comparing their contents.
 *
 * @param fpathA First file path
 * @param fpathB Second file path
 *
 * @return On success: `true` if the files are equal, `false` if they are not equal.
 * @return On failure: The error message from a failure to open or seek to the end of one of the files.
 */
[[nodiscard]] inline auto is_files_equal(const fs::path &fpathA, const fs::path &fpathB)
  -> outcome::result<bool, ErrorStr>
{
  // Adapted from: https://stackoverflow.com/a/37575457

  // Open the files and seek to the end
  std::ifstream fA(fpathA, std::ifstream::binary | std::ifstream::ate);
  std::ifstream fB(fpathB, std::ifstream::binary | std::ifstream::ate);

  // If there was a problem opening the files
  if (fA.fail() || fB.fail()) {
    std::string error_msg = "Error encountered opening and seeking to end of: ";
    if (fA.fail() && fB.fail()) {
      error_msg += fmt::format("{} and {}", fpathA.string(), fpathB.string());
    } else if (fA.fail()) {
      error_msg += fpathA.string();
    } else {
      error_msg += fpathB.string();
    }
    return outcome::failure(error_msg);
  }

  if (fA.tellg() != fB.tellg()) {
    return outcome::success(false);// size mismatch
  }

  // seek back to beginning and use std::equal to compare contents
  fA.seekg(0, std::ifstream::beg);
  fB.seekg(0, std::ifstream::beg);

  using buf_iter = std::istreambuf_iterator<char>;

  return outcome::success(std::equal(buf_iter(fA.rdbuf()), buf_iter(), buf_iter(fB.rdbuf())));
}

}// namespace io_util