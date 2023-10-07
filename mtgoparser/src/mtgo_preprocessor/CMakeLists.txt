find_package(fmt)

add_executable(mtgo_preprocesser main.cpp)


target_link_libraries(
  mtgo_preprocesser
  PRIVATE mtgoparser::mtgoparser_options
          mtgoparser::mtgoparser_warnings)

target_link_system_libraries(
  mtgo_preprocesser
  PRIVATE glaze::glaze
          fmt::fmt
          spdlog::spdlog
          rapidxml
          mtgoparser
          )

target_include_directories(mtgo_preprocesser PRIVATE "${CMAKE_BINARY_DIR}/configured_files/include")
include(../../cmake/Utilities.cmake)
copy_runtime_dlls(mtgo_preprocesser)
