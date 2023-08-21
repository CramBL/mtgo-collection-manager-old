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
add_executable(tests tests.cpp)
target_link_libraries(tests PRIVATE
                                    mtgoparser::mtgoparser_warnings
                                    #mtgoparser::mtgoparser_options
                                    #mtgoparser::mtgoparser_optionstest
                                    mtgoparser
                                    Catch2::Catch2WithMain)

message(VERBOSE "Is WIN32 and BUILD_SHARED_LIBS??")
if(WIN32 AND BUILD_SHARED_LIBS)
  message(VERBOSE "WIN32 and BUILD_SHARED_LIBS")
  add_custom_command(
    TARGET tests
    PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:tests> $<TARGET_FILE_DIR:tests>
    COMMAND_EXPAND_LISTS)
endif()


# automatically discover tests that are defined in catch based test files.
catch_discover_tests(
  tests
  TEST_PREFIX
  "unittests."
  REPORTER
  XML
  OUTPUT_DIR
  .
  OUTPUT_PREFIX
  "unittests."
  OUTPUT_SUFFIX
  .xml)