cmake_minimum_required(VERSION 3.18)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(
   mtgoparser
   VERSION 0.1.0
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