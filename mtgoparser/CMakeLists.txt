cmake_minimum_required(VERSION 3.18)

# Only set the cxx_standard if it is not set by someone else
if (NOT DEFINED CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 23)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED True)

# strongly encouraged to enable this globally to avoid conflicts between
# -Wpedantic being enabled and -std=c++20 and -std=gnu++20 for example
# when compiling with PCH enabled
set(CMAKE_CXX_EXTENSIONS OFF)

project(
   mtgoparser
   VERSION 0.1.0
   DESCRIPTION "MTGO collection manager app"
   HOMEPAGE_URL "%%https://github.com/CramBL/mtgo-collection-manager/%%"
   LANGUAGES CXX
)

file(GLOB srcs src/*.cpp include/*.hpp)

include(FetchContent)

FetchContent_Declare(
  glaze
  GIT_REPOSITORY https://github.com/stephenberry/glaze.git
  GIT_TAG main
  GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(glaze)

add_executable(${PROJECT_NAME} ${srcs})
target_include_directories(${PROJECT_NAME} PRIVATE include)
target_link_libraries(${PROJECT_NAME} PRIVATE glaze::glaze)

if (MSVC)
    add_compile_options(
      /Wall
      /WX
      )
else()
    add_compile_options(
      -fno-exceptions
      -Wall
      -Wextra
      -Wpedantic
      -Wshadow
      -Wdouble-promotion
      -Wformat=2
      -Wformat-truncation
      -Wundef
      -Wdeprecated
      -fno-common
      -fasynchronous-unwind-tables
      -Wl,-z,defs
      )
endif()