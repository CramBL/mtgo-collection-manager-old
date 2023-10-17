#pragma once

#include <mtgoparser/mtgo.hpp>
#include <string_view>

namespace mtgo_preprocessor::run {


/// Executed if the run command is set
[[nodiscard]] int run();


/// Executed if the run command and update option is set
[[nodiscard]] int update();

// Specifies the paths to the Goatbots card definition and price history JSON
struct GoatbotsPaths
{
  std::string_view card_defs_path;
  std::string_view price_hist_path;
};

/// Parse goatbots data from specified paths to card definition and price history JSON files.
[[nodiscard]] int parse_goatbots_data(mtgo::Collection &mtgo_collection, GoatbotsPaths paths);


}// namespace mtgo_preprocessor::run