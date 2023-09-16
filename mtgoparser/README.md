# MTGO Parser

## Purpose

Parse the various data formats produced by MTGO and the third party sources that provide price data etc.

## Get started

### Prerequisites

 - Either of: 
    - Clang 15+
    - GCC 12+
    - MSVC with C++20 support
 - cmake 3.20+

>working directory: mtgo-collection-manager/mtgoparser

 Generate build files
 ```shell
 cmake -S . -B ./build
 ```
 Compile MTGO Parser
 ```shell
 cmake --build build
 ```
 Run tests
 ```shell
cd build && ctest 
 ```