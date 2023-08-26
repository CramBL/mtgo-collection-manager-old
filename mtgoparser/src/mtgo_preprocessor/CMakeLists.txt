add_executable(mtgo_preprocesser main.cpp)

target_link_libraries(
  mtgo_preprocesser
  PRIVATE mtgoparser::mtgoparser_options
          mtgoparser::mtgoparser_warnings
          mtgoparser)