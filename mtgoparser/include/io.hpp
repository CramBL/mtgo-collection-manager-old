#pragma once

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace io_util {

[[nodiscard]] auto ReadFile(std::filesystem::path fpath) -> std::string
{
  // Open the stream to 'lock' the file.
  std::ifstream file(fpath, std::ios::in | std::ios::binary);

  // Obtain the size of the file.
  const auto fsize = std::filesystem::file_size(fpath);

  // Create a buffer.
  std::string str_buffer(fsize, '\0');

  // Read the whole file into the buffer.
  file.read(str_buffer.data(), fsize);

  return str_buffer;
}

[[nodiscard]] auto ReadFileCharBuf(std::filesystem::path fpath) -> std::vector<char>
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
  file.read(&char_buf[0], fsize);

  return char_buf;
}
}// namespace io_util