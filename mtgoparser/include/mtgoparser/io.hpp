#pragma once

#include <filesystem>
#include <fstream>
#include <ios>
#include <string>
#include <vector>

namespace io_util {

[[nodiscard]] inline auto ReadToStrBuf(const std::filesystem::path &fpath) -> std::string
{
  // Open the stream to 'lock' the file.
  std::ifstream file(fpath, std::ios::in | std::ios::binary);

  // Obtain the size of the file.
  const auto fsize = std::filesystem::file_size(fpath);

  // Create a buffer.
  std::string str_buffer(fsize, '\0');

  // Read the whole file into the buffer.
  file.read(str_buffer.data(), static_cast<std::streamsize>(fsize));

  return str_buffer;
}

[[nodiscard]] inline auto ReadToCharBuf(const std::filesystem::path &fpath) -> std::vector<char>
{
  // Open the stream to 'lock' the file.
  std::ifstream file(fpath, std::ios::in | std::ios::binary);

  // Obtain the size of the file.
  const auto fsize = std::filesystem::file_size(fpath);

  // Instantiate and pre-allocate
  std::vector<char> char_buf{};
  char_buf.resize(fsize + 1);// +1 for null termination

  // Null terminate (guaranteed to not be overwritten by the following call to read())
  char_buf[fsize] = '\0';

  // Read into buffer
  file.read(char_buf.data(), static_cast<std::streamsize>(fsize));

  return char_buf;
}
}// namespace io_util