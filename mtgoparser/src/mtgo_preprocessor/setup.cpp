#include "mtgo_preprocessor/setup.hpp"
#include "mtgo_preprocessor/config.hpp"


#include <mtgoparser/clap.hpp>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

namespace mtgo_preprocessor::setup {

int setup(std::vector<std::string_view> &args)
{
  // https://github.com/gabime/spdlog/wiki/0.-FAQ#switch-the-default-logger-to-stderr
  spdlog::set_default_logger(spdlog::stderr_color_st("rename_default_logger_to_keep_format"));
  spdlog::set_default_logger(spdlog::stderr_color_st(""));

  // Parse (and validate) command-line arguments
  if (auto errors = config::Config::get()->Parse(args)) {
    spdlog::error("{} arguments failed to validate", errors);
    return -1;
  };
  return 0;
}

}// namespace mtgo_preprocessor::setup