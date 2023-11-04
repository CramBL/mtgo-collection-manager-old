#include "mtgo_preprocessor/setup.hpp"
#include "mtgo_preprocessor/config.hpp"

#include <boost/outcome/result.hpp>
#include <boost/outcome/success_failure.hpp>

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <string>
#include <string_view>
#include <vector>

#include <fmt/core.h>

namespace mtgo_preprocessor::setup {

[[nodiscard]] auto setup(std::vector<std::string_view> &args) -> outcome::result<void, std::string>
{
  // https://github.com/gabime/spdlog/wiki/0.-FAQ#switch-the-default-logger-to-stderr
  spdlog::set_default_logger(spdlog::stderr_color_st("rename_default_logger_to_keep_format"));
  spdlog::set_default_logger(spdlog::stderr_color_st(""));

  // Parse (and validate) command-line arguments
  if (auto errors = config::Config::get()->Parse(args)) {
    return fmt::format("{} argument(s) failed to validate", errors);
  };
  return outcome::success();
}

}// namespace mtgo_preprocessor::setup