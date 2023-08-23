include(cmake/SystemLink.cmake)

cmake_minimum_required(VERSION 3.20) # LLVM 16.0.0 requires >= 3.20.0



# Only set the cxx_standard if it is not set by someone else
if (NOT DEFINED CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 20)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED True)

# strongly encouraged to enable this globally to avoid conflicts between
# -Wpedantic being enabled and -std=c++20 and -std=gnu++20 for example
# when compiling with PCH enabled
set(CMAKE_CXX_EXTENSIONS OFF)

project(
   mtgoparser
   VERSION 0.1.0
   DESCRIPTION "Parser library for the MTGO collection manager app"
   HOMEPAGE_URL "https://github.com/CramBL/mtgo-collection-manager/mtgoparser"
   LANGUAGES CXX
)

file(GLOB srcs src/*.cpp include/*.hpp)

# Define the library target
add_library(mtgoparser INTERFACE)
target_include_directories(mtgoparser INTERFACE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_link_system_library(${PROJECT_NAME} INTERFACE glaze::glaze)
target_link_system_library(${PROJECT_NAME} INTERFACE spdlog::spdlog)
target_link_system_library(${PROJECT_NAME} INTERFACE rapidxml)


include(cmake/PreventInSourceBuilds.cmake)
include(ProjectOptions.cmake)

mtgoparser_setup_options()

mtgoparser_global_options()
include(Dependencies.cmake)
mtgoparser_setup_dependencies()

mtgoparser_local_options()

# don't know if this should be set globally from here or not...
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

set(GIT_SHA
    "Unknown"
    CACHE STRING "SHA this build was generated from")
string(
  SUBSTRING "${GIT_SHA}"
            0
            8
            GIT_SHORT_SHA)

target_compile_features(mtgoparser_options INTERFACE cxx_std_${CMAKE_CXX_STANDARD})


add_library(mtgoparser::mtgoparser_options ALIAS mtgoparser_options)
add_library(mtgoparser::mtgoparser_warnings ALIAS mtgoparser_warnings)

#add_library(mtgoparser::mtgoparser_options INTERFACE IMPORTED)
#add_library(mtgoparser::mtgoparser_warnings INTERFACE IMPORTED)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${PROJECT_BINARY_DIR}/mtgoparserConfigVersion.cmake"
    VERSION 0.1
    COMPATIBILITY AnyNewerVersion
)
install(TARGETS spdlog rapidxml fmt mtgoparser
#install(TARGETS mtgoparser
    EXPORT mtgoparserTargets
    LIBRARY DESTINATION lib COMPONENT Runtime
    ARCHIVE DESTINATION lib COMPONENT Development
    RUNTIME DESTINATION bin COMPONENT Runtime
    PUBLIC_HEADER DESTINATION include COMPONENT Development
    BUNDLE DESTINATION bin COMPONENT Runtime
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
    "${PROJECT_SOURCE_DIR}/cmake/mtgoparserConfig.cmake.in"
    "${PROJECT_BINARY_DIR}/mtgoparserConfig.cmake"
    INSTALL_DESTINATION lib/cmake/mtgoparser
)

install(EXPORT mtgoparserTargets DESTINATION lib/cmake/mtgoparser)
install(FILES "${PROJECT_BINARY_DIR}/mtgoparserConfigVersion.cmake"
              "${PROJECT_BINARY_DIR}/mtgoparserConfig.cmake"
        DESTINATION lib/cmake/mtgoparser)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/ DESTINATION include)


# Don't even look at tests if we're not top level
if(NOT PROJECT_IS_TOP_LEVEL)
  return()
endif()

# Adding the tests:

list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/extras)
find_package(Catch2 3 REQUIRED)
include(CTest)
include(Catch)

if(BUILD_TESTING)
  message(AUTHOR_WARNING "Building Tests. Be sure to check out test/constexpr_tests.cpp for constexpr testing")
  add_subdirectory(test)
endif()

# If MSVC is being used, and ASAN is enabled, we need to set the debugger environment
# so that it behaves well with MSVC's debugger, and we can run the target from visual studio
if(MSVC)
  get_all_installable_targets(all_targets)
  message("all_targets=${all_targets}")
  set_target_properties(${all_targets} PROPERTIES VS_DEBUGGER_ENVIRONMENT "PATH=$(VC_ExecutablePath_x64);%PATH%")
endif()

# set the startup project for the "play" button in MSVC
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT mtgoparser)

if(CMAKE_SKIP_INSTALL_RULES)
  return()
endif()

## Not ready
include(cmake/PackageProject.cmake)

# Add other targets here, by default we just package one executable
#mtgoparser_package_project(
#  TARGETS
#  intro
#  mtgoparser_options
#  mtgoparser_warnings
  # FIXME: this does not work! CK
  # PRIVATE_DEPENDENCIES_CONFIGURED project_options project_warnings
#)

# Experience shows that explicit package naming can help make it easier to sort
# out potential ABI related issues before they start, while helping you
# track a build to a specific GIT SHA
set(CPACK_PACKAGE_FILE_NAME
   "${CMAKE_PROJECT_NAME}-${CMAKE_PROJECT_VERSION}-${GIT_SHORT_SHA}-${CMAKE_SYSTEM_NAME}-${CMAKE_BUILD_TYPE}-${CMAKE_CXX_COMPILER_ID}-${CMAKE_CXX_COMPILER_VERSION}"
)

include(CPack)
