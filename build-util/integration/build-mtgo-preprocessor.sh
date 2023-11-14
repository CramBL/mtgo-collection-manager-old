#!/bin/sh

# This script is used to build the MTGO Preprocessor binary for release, ready to be included in the MTGO GUI application.

file_boost_exclude_libs_list="build-util/boost-exclude-libs.txt"
boost_exclude_libs=$(cat ${file_boost_exclude_libs_list})

println_red() {
    printf "\e[31m%b\e[0m\n" "${1}"
}
println_yellow() {
    printf "\e[33m%b\e[0m\n" "${1}"
}
println_cyan() {
    printf "\e[96m%b\e[0m\n" "${1}"
}

# Get the operating system name
os_name=$(uname -s)

# Set variable based on the operating system
if [ "$os_name" = "Linux" ]; then
    user_linker="On"
    linker="mold"
    ipo="On"
elif [ "$os_name" = "Darwin" ]; then
    user_linker="Off"
    linker="default" # Does nothing when user linker is set to Off
    ipo="Off"
else
    println_red "Unsupported operating system: $os_name"
    exit 1
fi

# Build MTGO Preprocessor
configure_cmake="cd mtgoparser && cmake -S . -B build \
-G \"Ninja Multi-Config\" \
-Dmtgoparser_DEPLOYING_BINARY=On \
-Dmtgoparser_ENABLE_IPO=${ipo} \
-DCMAKE_BUILD_TYPE:STRING=Release \
-Dmtgoparser_ENABLE_COVERAGE:BOOL=OFF \
-DBOOST_EXCLUDE_LIBRARIES=\"${boost_exclude_libs}\" \
-Dmtgoparser_WARNINGS_AS_ERRORS:BOOL=OFF \
-Dmtgoparser_ENABLE_CPPCHECK:BOOL=OFF \
-DUSER_LINKER_OPTION=${linker} \
-Dmtgoparser_ENABLE_USER_LINKER:BOOL=${user_linker}"

println_cyan "Configuring cmake with command:"
println_yellow "${configure_cmake}\n"

build_mtgo_preprocessor="cmake --build build --config Release"

println_cyan "Building MTGO Preprocessor with command:"
println_yellow "${build_mtgo_preprocessor}\n"

# Run the commands
eval "$configure_cmake && $build_mtgo_preprocessor"
