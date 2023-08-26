include(../cmake/SystemLink.cmake)
cmake_minimum_required(VERSION 3.15)

project(mtgoparserTests LANGUAGES CXX)

# ---- Test as standalone project the exported config package ----
if(PROJECT_IS_TOP_LEVEL OR TEST_INSTALLED_VERSION)

  enable_testing()

  find_package(mtgoparser CONFIG REQUIRED) # for project_options, ...

  if(NOT TARGET mtgoparser_options)
    message(FATAL_ERROR "Required config package not found!")
    return()
  endif()
endif()

# ---- Dependencies ----

list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/extras)
find_package(Catch2 3 REQUIRED)
include(Catch)
add_executable(test_xml_parse test_xml_parse.cpp)
target_link_libraries(test_xml_parse PRIVATE
                                    mtgoparser::mtgoparser_warnings
                                    mtgoparser::mtgoparser_options
                                    mtgoparser)
target_link_system_library(test_xml_parse PRIVATE Catch2::Catch2WithMain)


if(WIN32 AND BUILD_SHARED_LIBS)
  message(VERBOSE "WIN32 and BUILD_SHARED_LIBS")
  add_custom_command(
    TARGET test_xml_parse
    PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:test_xml_parse> $<TARGET_FILE_DIR:test_xml_parse>
    COMMAND_EXPAND_LISTS)
endif()


# automatically discover tests that are defined in catch based test files.
catch_discover_tests(
  test_xml_parse
  TEST_PREFIX
  "test_xml_parse."
  REPORTER
  XML
  OUTPUT_DIR
  .
  OUTPUT_PREFIX
  "test_xml_parse."
  OUTPUT_SUFFIX
  .xml)

# Add benchmarks for XML parsing
add_executable(benchmark_xml_parse benchmark_xml_parse.cpp)
target_link_libraries(
  benchmark_xml_parse
  PRIVATE mtgoparser::mtgoparser_warnings
          mtgoparser::mtgoparser_options
          mtgoparser
          Catch2::Catch2WithMain)

catch_discover_tests(
  benchmark_xml_parse
  TEST_PREFIX
  "benchmark_xml_parse."
  REPORTER
  XML
  OUTPUT_DIR
  .
  OUTPUT_PREFIX
  "benchmark_xml_parse."
  OUTPUT_SUFFIX
  .xml)

# JSON parsing tests
add_executable(test_json_parse test_json_parse.cpp)
target_link_libraries(test_json_parse PRIVATE
                                    mtgoparser::mtgoparser_warnings
                                    mtgoparser::mtgoparser_options
                                    mtgoparser)
target_link_system_library(test_json_parse PRIVATE Catch2::Catch2WithMain)


if(WIN32 AND BUILD_SHARED_LIBS)
  message(VERBOSE "WIN32 and BUILD_SHARED_LIBS")
  add_custom_command(
    TARGET test_json_parse
    PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:test_json_parse> $<TARGET_FILE_DIR:test_json_parse>
    COMMAND_EXPAND_LISTS)
endif()


# automatically discover tests that are defined in catch based test files.
catch_discover_tests(
  test_json_parse
  TEST_PREFIX
  "test_json_parse."
  REPORTER
  XML
  OUTPUT_DIR
  .
  OUTPUT_PREFIX
  "test_json_parse."
  OUTPUT_SUFFIX
  .xml)