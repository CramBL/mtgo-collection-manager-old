#!/bin/sh

# This script is used to build each of the components of the project as effeciently as possible and as they should be built for release,
#  ready to be included in the MTGO GUI application.

println_red() {
    printf "\e[31m%b\e[0m\n" "${1}"
}
println_green() {
    printf "\e[32m%b\e[0m\n" "${1}"
}
println_magenta() {
    printf "\e[95m%b\e[0m\n" "${1}"
}

# String to store process IDs (would be an array in bash, but this is POSIX sh!)
pids=""

# Function to run a command in the background
spawn_child_proc() {
    println_magenta "Running command: "
    println_magenta "$@"
    sh "$@" &
    PID=$!
    println_magenta "Running process with PID: $PID"
    # Store the process ID in the string
    pids="$pids $PID"
}

# Build MTGO Getter
spawn_child_proc ./build-util/integration/build-mtgogetter.sh
# Build MTGO Preprocessor
spawn_child_proc ./build-util/integration/build-mtgo-preprocessor.sh

# Exit code to be returned at the end of the script
final_exit_code=0

for pid in $pids; do
    wait "$pid" # Wait for each process to finish
    exit_code=$?
    if [ $exit_code -ne 0 ]; then
        println_red "ERROR: Process with PID $pid exited with code $exit_code -- Integration build failing"
        final_exit_code=$exit_code
    else
        println_green "Process with PID $pid exited successfully"
    fi
done

if [ $final_exit_code -eq 0 ]; then
    println_green "Integration build successful"
else
    println_red "ERROR: Integration build failed"
fi

exit $final_exit_code
