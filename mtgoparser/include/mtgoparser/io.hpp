#pragma once

// NOLINTBEGIN
#include <toml++/toml.hpp>
// NOLINTEND

#include <boost/outcome.hpp>
#include <boost/outcome/result.hpp>

#include <fmt/core.h>

#include <filesystem>
#include <fstream>
#include <ios>
#include <string>
#include <vector>

namespace io_util {

namespace outcome = BOOST_OUTCOME_V2_NAMESPACE;
namespace fs = std::filesystem;

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
 * @brief Save a string buffer to a file with an ISO 8601 timestamp appended to the file name.
 *
 * Any directories in the path that do not exist will be created.
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
[[nodiscard]] inline auto save_with_timestamp(auto buf, const fs::path &fpath, auto ext)
  -> outcome::result<fs::path, std::string>
{
  try {
    // If the path is not to the current directory
    if (fpath.has_parent_path()) {
      // Create directories if they don't exist
      fs::create_directories(fpath.parent_path());
    }
    // Get the current time
    auto now = std::chrono::system_clock::now();
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);

    // Convert to a tm struct in UTC (std::gmtime converts to UTC) and %Y-%m-%dT%H-%M-%SZ is ISO 8601
    const auto now_utc_iso8601 = std::put_time(std::gmtime_s(&now_time_t), "%Y-%m-%dT%H-%M-%SZ");

    // Convert the tm struct to a string
    std::ostringstream ss;
    ss << now_utc_iso8601;
    std::string timestamp = ss.str();

    fs::path fpath_with_time = fpath.parent_path() / fmt::format("{}_{}.{}", fpath.stem().string(), timestamp, ext);

    // Open the file
    std::ofstream file(fpath_with_time, std::ios::out | std::ios::binary);

    // Write the buffer to the file
    file.write(buf.data(), static_cast<std::streamsize>(buf.size()));

    // Return the file name
    return outcome::success(fpath_with_time);

  } catch (const std::exception &e) {
    return outcome::failure(fmt::format("Failed to save file from path: {}, err: {}", fpath.string(), e.what()));
  }
}

}// namespace io_util