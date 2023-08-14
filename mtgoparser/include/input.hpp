#include <filesystem>
#include <fstream>
#include <string>

namespace fs = std::filesystem;

// Use version that checks for cached size?
// https://en.cppreference.com/w/cpp/filesystem/directory_entry/file_size

[[nodiscard]] std::string readFile(fs::path path)
{
    // Open the stream to 'lock' the file.
    std::ifstream f(path, std::ios::in | std::ios::binary);

    // Obtain the size of the file.
    const auto sz = fs::file_size(path);

    // Create a buffer.
    std::string buffer(sz, '\0');

    // Read the whole file into the buffer.
    f.read(buffer.data(), sz);

    return buffer;
}