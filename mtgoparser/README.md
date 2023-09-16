# MTGO Parser

## Purpose

Parse the various data formats produced by MTGO and the third party sources that provide price data etc.

## Get started

### Prerequisites

 - **Compiler** any of: 
    - Clang 15+
    - GCC 12+
    - MSVC with C++20 support
 - cmake 3.20+
 - **Generator** any of:
    - "Ninja Multi-Config"
    - "Visual Studio 17 2022"
    - "Unix Makefiles" (Not recommended)

>working directory: mtgo-collection-manager/mtgoparser

 Generate build files
 ```shell
 cmake -S . -B ./build -G "Ninja Multi-Config"
 ```
 Compile MTGO Parser
 ```shell
 cmake --build build
 ```
 Run tests
 ```shell
cd build && ctest 
 ```