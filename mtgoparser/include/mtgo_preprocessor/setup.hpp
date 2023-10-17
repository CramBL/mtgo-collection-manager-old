#pragma once

#include <string_view>
#include <vector>

namespace mtgo_preprocessor::setup {

/// Initiate logger and config
///
/// returns 0 on success
int setup(std::vector<std::string_view> &args);


}// namespace mtgo_preprocessor::setup