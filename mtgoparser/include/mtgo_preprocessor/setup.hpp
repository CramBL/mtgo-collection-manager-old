#pragma once

#include <string_view>
#include <vector>

#include <boost/outcome.hpp>
#include <boost/outcome/result.hpp>

namespace mtgo_preprocessor::setup {


namespace outcome = BOOST_OUTCOME_V2_NAMESPACE;

/// Initiate logger and config.
///
/// If `setup` fails, returns a string containing an error message
auto setup(std::vector<std::string_view> &args) -> outcome::result<void, std::string>;


}// namespace mtgo_preprocessor::setup