#pragma once

#include <mtgoparser/mtgo.hpp>

#include <boost/outcome.hpp>
#include <boost/outcome/result.hpp>

#include <string_view>

namespace mtgo_preprocessor::run {

namespace outcome = BOOST_OUTCOME_V2_NAMESPACE;

/**
 * @brief Entry point when `MTGO Preprocessor` is run with the `run` command.
 *
 * @return int
 */
[[nodiscard]] int run();


/**
 * @brief Update the an MTGO card collection with the latest data.
 *
 *  Parses the MTGO collection XML and extracts information from:
 *    - Scryfall card information JSON
 *    - Goatbots card definition JSON
 *    - Goatbots price history JSON
 *
 * @return int
 *
 * @note Executed if the run command and update option is set.
 */
[[nodiscard]] int update();

/**
 * @brief Specifies the paths to the Goatbots card definition and price history JSON files.
 *
 * @param card_defs_path The path to the Goatbots card definition JSON file
 * @param price_hist_path The path to the Goatbots price history JSON file
 * @return GoatbotsPaths
 *
 * @note The paths are specified as `std::string_view` to avoid copying the strings.
 */
struct GoatbotsPaths
{
  std::string_view card_defs_path;
  std::string_view price_hist_path;
};

using Success = void;
using ErrorStr = std::string;

/**
 * @brief Parse goatbots data from specified paths to card definition and price history JSON files.
 *
 * @param mtgo_collection The MTGO collection to parse Goatbots data into
 * @param paths The paths to the Goatbots card definition and price history JSON files
 * @return On success: `Success` (void)
 * @return On failure: `ErrorStr` (std::string)
 */
[[nodiscard]] auto parse_goatbots_data(mtgo::Collection &mtgo_collection, GoatbotsPaths paths)
  -> outcome::result<Success, ErrorStr>;


}// namespace mtgo_preprocessor::run