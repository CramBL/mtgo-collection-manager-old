include(cmake/SystemLink.cmake)
include(cmake/LibFuzzer.cmake)
include(CMakeDependentOption)
include(CheckCXXCompilerFlag)


macro(mtgoparser_supports_sanitizers)
  if((CMAKE_CXX_COMPILER_ID MATCHES ".*Clang.*" OR CMAKE_CXX_COMPILER_ID MATCHES ".*GNU.*") AND NOT WIN32)
    set(SUPPORTS_UBSAN ON)
  else()
    set(SUPPORTS_UBSAN OFF)
  endif()

  if((CMAKE_CXX_COMPILER_ID MATCHES ".*Clang.*" OR CMAKE_CXX_COMPILER_ID MATCHES ".*GNU.*") AND WIN32)
    set(SUPPORTS_ASAN OFF)
  else()
    set(SUPPORTS_ASAN ON)
  endif()
endmacro()

macro(mtgoparser_setup_options)
  option(mtgoparser_ENABLE_HARDENING "Enable hardening" ON)
  option(mtgoparser_ENABLE_COVERAGE "Enable coverage reporting" OFF)
  cmake_dependent_option(
    mtgoparser_ENABLE_GLOBAL_HARDENING
    "Attempt to push hardening options to built dependencies"
    ON
    mtgoparser_ENABLE_HARDENING
    OFF)

  mtgoparser_supports_sanitizers()

  if(NOT PROJECT_IS_TOP_LEVEL OR mtgoparser_PACKAGING_MAINTAINER_MODE)
    option(mtgoparser_ENABLE_IPO "Enable IPO/LTO" OFF)
    option(mtgoparser_WARNINGS_AS_ERRORS "Treat Warnings As Errors" OFF)
    option(mtgoparser_ENABLE_USER_LINKER "Enable user-selected linker" OFF)
    option(mtgoparser_ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" OFF)
    option(mtgoparser_ENABLE_SANITIZER_LEAK "Enable leak sanitizer" OFF)
    option(mtgoparser_ENABLE_SANITIZER_UNDEFINED "Enable undefined sanitizer" OFF)
    option(mtgoparser_ENABLE_SANITIZER_THREAD "Enable thread sanitizer" OFF)
    option(mtgoparser_ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" OFF)
    option(mtgoparser_ENABLE_UNITY_BUILD "Enable unity builds" OFF)
    option(mtgoparser_ENABLE_CLANG_TIDY "Enable clang-tidy" OFF)
    option(mtgoparser_ENABLE_CPPCHECK "Enable cpp-check analysis" OFF)
    option(mtgoparser_ENABLE_PCH "Enable precompiled headers" OFF)
    option(mtgoparser_ENABLE_CACHE "Enable ccache" OFF)
  elseif(mtgoparser_DEPLOYING_BINARY)
    option(mtgoparser_ENABLE_IPO "Enable IPO/LTO" ON)
    option(mtgoparser_WARNINGS_AS_ERRORS "Treat Warnings As Errors" ON)
    option(mtgoparser_ENABLE_USER_LINKER "Enable user-selected linker" OFF)
    option(mtgoparser_ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" OFF)
    option(mtgoparser_ENABLE_SANITIZER_LEAK "Enable leak sanitizer" OFF)
    option(mtgoparser_ENABLE_SANITIZER_UNDEFINED "Enable undefined sanitizer" OFF)
    option(mtgoparser_ENABLE_SANITIZER_THREAD "Enable thread sanitizer" OFF)
    option(mtgoparser_ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" OFF)
    option(mtgoparser_ENABLE_UNITY_BUILD "Enable unity builds" OFF)
    option(mtgoparser_ENABLE_CLANG_TIDY "Enable clang-tidy" ON)
    option(mtgoparser_ENABLE_CPPCHECK "Enable cpp-check analysis" ON)
    option(mtgoparser_ENABLE_PCH "Enable precompiled headers" OFF)
    option(mtgoparser_ENABLE_CACHE "Enable ccache" ON)
  else()
    option(mtgoparser_ENABLE_IPO "Enable IPO/LTO" ON)
    option(mtgoparser_WARNINGS_AS_ERRORS "Treat Warnings As Errors" ON)
    option(mtgoparser_ENABLE_USER_LINKER "Enable user-selected linker" OFF)
    option(mtgoparser_ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" ${SUPPORTS_ASAN})
    option(mtgoparser_ENABLE_SANITIZER_LEAK "Enable leak sanitizer" OFF)
    option(mtgoparser_ENABLE_SANITIZER_UNDEFINED "Enable undefined sanitizer" ${SUPPORTS_UBSAN})
    option(mtgoparser_ENABLE_SANITIZER_THREAD "Enable thread sanitizer" OFF)
    option(mtgoparser_ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" OFF)
    option(mtgoparser_ENABLE_UNITY_BUILD "Enable unity builds" OFF)
    option(mtgoparser_ENABLE_CLANG_TIDY "Enable clang-tidy" ON)
    option(mtgoparser_ENABLE_CPPCHECK "Enable cpp-check analysis" ON)
    option(mtgoparser_ENABLE_PCH "Enable precompiled headers" OFF)
    option(mtgoparser_ENABLE_CACHE "Enable ccache" ON)
  endif()

  if(NOT PROJECT_IS_TOP_LEVEL)
    mark_as_advanced(
      mtgoparser_ENABLE_IPO
      mtgoparser_WARNINGS_AS_ERRORS
      mtgoparser_ENABLE_USER_LINKER
      mtgoparser_ENABLE_SANITIZER_ADDRESS
      mtgoparser_ENABLE_SANITIZER_LEAK
      mtgoparser_ENABLE_SANITIZER_UNDEFINED
      mtgoparser_ENABLE_SANITIZER_THREAD
      mtgoparser_ENABLE_SANITIZER_MEMORY
      mtgoparser_ENABLE_UNITY_BUILD
      mtgoparser_ENABLE_CLANG_TIDY
      mtgoparser_ENABLE_CPPCHECK
      mtgoparser_ENABLE_COVERAGE
      mtgoparser_ENABLE_PCH
      mtgoparser_ENABLE_CACHE)
  endif()

  mtgoparser_check_libfuzzer_support(LIBFUZZER_SUPPORTED)
  if(LIBFUZZER_SUPPORTED AND (mtgoparser_ENABLE_SANITIZER_ADDRESS OR mtgoparser_ENABLE_SANITIZER_THREAD OR mtgoparser_ENABLE_SANITIZER_UNDEFINED))
    set(DEFAULT_FUZZER ON)
  else()
    set(DEFAULT_FUZZER OFF)
  endif()

  option(mtgoparser_BUILD_FUZZ_TESTS "Enable fuzz testing executable" ${DEFAULT_FUZZER})

endmacro()

macro(mtgoparser_global_options)
  if(mtgoparser_ENABLE_IPO)
    include(cmake/InterproceduralOptimization.cmake)
    mtgoparser_enable_ipo()
  endif()

  mtgoparser_supports_sanitizers()

  if(mtgoparser_ENABLE_HARDENING AND mtgoparser_ENABLE_GLOBAL_HARDENING)
    include(cmake/Hardening.cmake)
    if(NOT SUPPORTS_UBSAN
       OR mtgoparser_ENABLE_SANITIZER_UNDEFINED
       OR mtgoparser_ENABLE_SANITIZER_ADDRESS
       OR mtgoparser_ENABLE_SANITIZER_THREAD
       OR mtgoparser_ENABLE_SANITIZER_LEAK)
      set(ENABLE_UBSAN_MINIMAL_RUNTIME FALSE)
    else()
      set(ENABLE_UBSAN_MINIMAL_RUNTIME TRUE)
    endif()
    message("${mtgoparser_ENABLE_HARDENING} ${ENABLE_UBSAN_MINIMAL_RUNTIME} ${mtgoparser_ENABLE_SANITIZER_UNDEFINED}")
    mtgoparser_enable_hardening(mtgoparser_options ON ${ENABLE_UBSAN_MINIMAL_RUNTIME})
  endif()
endmacro()

macro(mtgoparser_local_options)
  if(PROJECT_IS_TOP_LEVEL)
    include(cmake/StandardProjectSettings.cmake)
  endif()

  add_library(mtgoparser_warnings INTERFACE)
  add_library(mtgoparser_options INTERFACE)

  include(cmake/CompilerWarnings.cmake)
  mtgoparser_set_project_warnings(
    mtgoparser_warnings
    ${mtgoparser_WARNINGS_AS_ERRORS}
    ""
    ""
    "")

  if(mtgoparser_ENABLE_USER_LINKER)
    include(cmake/Linker.cmake)
    configure_linker(mtgoparser_options)
  endif()

  include(cmake/Sanitizers.cmake)
  mtgoparser_enable_sanitizers(
    mtgoparser_options
    ${mtgoparser_ENABLE_SANITIZER_ADDRESS}
    ${mtgoparser_ENABLE_SANITIZER_LEAK}
    ${mtgoparser_ENABLE_SANITIZER_UNDEFINED}
    ${mtgoparser_ENABLE_SANITIZER_THREAD}
    ${mtgoparser_ENABLE_SANITIZER_MEMORY})

  set_target_properties(mtgoparser_options PROPERTIES UNITY_BUILD ${mtgoparser_ENABLE_UNITY_BUILD})

  if(mtgoparser_ENABLE_PCH)
    target_precompile_headers(
      mtgoparser_options
      INTERFACE
      <vector>
      <string>
      <utility>)
  endif()

  if(mtgoparser_ENABLE_CACHE)
    include(cmake/Cache.cmake)
    mtgoparser_enable_cache()
  endif()

  include(cmake/StaticAnalyzers.cmake)
  if(mtgoparser_ENABLE_CLANG_TIDY)
    mtgoparser_enable_clang_tidy(mtgoparser_options ${mtgoparser_WARNINGS_AS_ERRORS})
  endif()

  if(mtgoparser_ENABLE_CPPCHECK)
    mtgoparser_enable_cppcheck(${mtgoparser_WARNINGS_AS_ERRORS} "" # override cppcheck options
    )
  endif()

  if(mtgoparser_ENABLE_COVERAGE)
    include(cmake/Tests.cmake)
    mtgoparser_enable_coverage(mtgoparser_options)
  endif()

  if(mtgoparser_WARNINGS_AS_ERRORS)
    check_cxx_compiler_flag("-Wl,--fatal-warnings" LINKER_FATAL_WARNINGS)
    if(LINKER_FATAL_WARNINGS)
      # This is not working consistently, so disabling for now
      # target_link_options(mtgoparser_options INTERFACE -Wl,--fatal-warnings)
    endif()
  endif()

  if(mtgoparser_ENABLE_HARDENING AND NOT mtgoparser_ENABLE_GLOBAL_HARDENING)
    include(cmake/Hardening.cmake)
    if(NOT SUPPORTS_UBSAN
       OR mtgoparser_ENABLE_SANITIZER_UNDEFINED
       OR mtgoparser_ENABLE_SANITIZER_ADDRESS
       OR mtgoparser_ENABLE_SANITIZER_THREAD
       OR mtgoparser_ENABLE_SANITIZER_LEAK)
      set(ENABLE_UBSAN_MINIMAL_RUNTIME FALSE)
    else()
      set(ENABLE_UBSAN_MINIMAL_RUNTIME TRUE)
    endif()
    mtgoparser_enable_hardening(mtgoparser_options OFF ${ENABLE_UBSAN_MINIMAL_RUNTIME})
  endif()

endmacro()
