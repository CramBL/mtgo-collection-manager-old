#pragma once

#include <filesystem>
#include <fstream>
#include <string>

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
}// namespace io_util