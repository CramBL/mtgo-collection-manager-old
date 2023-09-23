param (
    [string]$target = "default",
    [string]$MTGOPARSER_IPO = "On",
    [string]$MTGOPARSER_GENERATOR = "Ninja Multi-Config"
)

# Default flags
$MTGOPARSER_BUILD_MODE = "Release"
$MTGOPARSER_ENABLE_COV = $false

# Minimum supported versions
$RUST_MIN_VERSION = "1.70.0"
$GO_MIN_VERSION = "1.21"
$CMAKE_MIN_VERSION = "3.20"
$GCC_MIN_VERSION = "12.0.0"
$LLVM_MIN_VERSION = "15.0.3"

# Get version information from the shell
try {
    $RUST_VERSION = & rustc --version | ForEach-Object { $_.Split(" ")[1] }
} catch {
    $RUST_VERSION = "NOT FOUND"
}
try {
    $GO_VERSION = & go version | ForEach-Object { $_.Split(" ")[2] -replace "go", "" }
} catch {
    $GO_VERSION = "NOT FOUND"
}
try {
    $CMAKE_VERSION = & cmake --version | ForEach-Object { $_.Split(" ")[2] -replace "[\r\n]", "" } | Select-Object -First 1
} catch {
    $CMAKE_VERSION = "NOT FOUND"
}
try {
    $CLANG_VERSION = & clang --version | ForEach-Object { $_.Split(" ")[3] -replace "[\r\n]", "" }
} catch {
    $CLANG_VERSION = "NOT FOUND"
}
try {
    $GCC_VERSION = & gcc --version | Select-String -Pattern '\d+\.\d+\.\d+' | ForEach-Object { $_.Matches[0].Value }
} catch {
    $GCC_VERSION = "NOT FOUND"
}
try {
    $OS_INFO = Get-CimInstance Win32_OperatingSystem
    $OS_TYPE = $OS_INFO.Caption
} catch {
    $OS_TYPE = "UNKNOWN"
}


function Test-GoInstallation {
    if ($GO_VERSION -eq "NOT FOUND") {
        Write-Host "Error: Go is not installed. Please install Go and try again."
        exit 1
    }
}

function Test-RustInstallation {
    if ($RUST_VERSION -eq "NOT FOUND") {
        Write-Host "Error: Rust is not installed. Please install Rust and try again."
        exit 1
    }
}


function Show-Versions {
    Write-Host "Operating System: $OS_TYPE"
    Write-Host "Rust : $RUST_VERSION (min. $RUST_MIN_VERSION)"
    Write-Host "Go   : $GO_VERSION (min. $GO_MIN_VERSION)"
    Write-Host "C++"
    Write-Host "  - LLVM: $CLANG_VERSION (min. $LLVM_MIN_VERSION)"
    Write-Host "  - GCC : $GCC_VERSION (min. $GCC_MIN_VERSION)"
    Write-Host "CMake: $CMAKE_VERSION (min. $CMAKE_MIN_VERSION)"
    Write-Host "CMake generator: ${MTGOPARSER_GENERATOR}"
    if ($MTGOPARSER_GENERATOR -eq "Ninja Multi-Config") {
        try {
            $NINJA_VERSION = & ninja --version
        } catch {
            Write-Host "Ninja Multi-Config specified but Ninja not found"
            exit 1
        }
        Write-Host "Ninja: ${NINJA_VERSION}"
    }
}

function Build-All {
    Write-Host "----------------------------------"
    Write-Host "==> Building all targets "
    Write-Host "----------------------------------"
    Build-Mtgogetter
    Build-Mtgoparser
    Build-Mtgoupdater
    Write-Host "================================= "
    Write-Host "=== Done building all targets === "
    Write-Host "================================= "
}

# For integration testing, disabling warnings as errors for mtgoparser
function Build-AllIntegration {
    Build-Mtgogetter
    Build-MtgoparserIntegration
    Build-Mtgoupdater
}

function Test-All {
    Write-Host "----------------------------------"
    Write-Host "==> Testing all targets "
    Write-Host "----------------------------------"
    Test-Mtgogetter
    Test-Mtgoparser
    Test-Mtgoupdater
    Write-Host "================================= "
    Write-Host "=== Done testing all targets === "
    Write-Host "================================= "
}

function Build-Mtgoparser {
    Show-Versions
    Write-Host "==> Building MTGO Parser..."
    Set-Location mtgoparser
    cmake -S . -B build -G "${MTGOPARSER_GENERATOR}" -Dmtgoparser_ENABLE_IPO="${MTGOPARSER_IPO}" -DCMAKE_BUILD_TYPE:STRING=${MTGOPARSER_BUILD_MODE} -Dmtgoparser_ENABLE_COVERAGE:BOOL=${MTGOPARSER_ENABLE_COV} 2>&1
    if ($LASTEXITCODE -ne 0) {
        Write-Host "!!! ERROR while setting up build files for MTGO Parser: code ${LASTEXITCODE}"
    } else {
        cmake --build build --config ${MTGOPARSER_BUILD_MODE}
        if ($LASTEXITCODE -ne 0) {
            Write-Host "!!! ERROR while building MTGO Parser: code ${LASTEXITCODE}"
        } else {
            Write-Host "=== Done building MTGO Parser ==="
        }
    }
    Set-Location ..

}

function Build-MtgoparserIntegration {
    Show-Versions
    Write-Host "==> Building MTGO Parser..."
    Set-Location mtgoparser
    cmake -S . -B build -G "${MTGOPARSER_GENERATOR}" -Dmtgoparser_ENABLE_IPO="${MTGOPARSER_IPO}" -DCMAKE_BUILD_TYPE:STRING=${MTGOPARSER_BUILD_MODE} -Dmtgoparser_ENABLE_COVERAGE:BOOL=${MTGOPARSER_ENABLE_COV} -Dmtgoparser_WARNINGS_AS_ERRORS:BOOL=OFF -Dmtgoparser_ENABLE_CLANG_TIDY:BOOL=OFF -Dmtgoparser_ENABLE_CPPCHECK:BOOL=OFF
    cmake --build build --config ${MTGOPARSER_BUILD_MODE}
    Set-Location ..
    Write-Host "=== Done building MTGO Parser ==="
}

function Test-Mtgoparser {
    Show-Versions
    Write-Host "==> Testing MTGO Parser..."
    Set-Location mtgoparser\build
    ctest
    Set-Location -Path $PSScriptRoot
    Write-Host "=== Done testing MTGO Parser ==="
}

function Test-MtgoparserBenchmark {
    Show-Versions
    Write-Host "==> Running MTGO Parser benchmarks..."
    Set-Location mtgoparser\build\test
    .\Release\benchmark_xml_parse.exe [.]
    Set-Location -Path $PSScriptRoot
    Write-Host "=== Done running MTGO Parser benchmarks ==="
}

function Build-Mtgogetter {
    Show-Versions
    Test-GoInstallation
    Write-Host "==> Building MTGO Getter..."
    go build -C mtgogetter -v
    Write-Host "=== Done building MTGO Getter ==="
}

function Test-Mtgogetter {
    Show-Versions
    Test-GoInstallation
    Write-Host "==> Testing MTGO Getter..."
    go test -C mtgogetter -v ./...
    Write-Host "=== Done testing MTGO Getter ==="
}

function Build-Mtgoupdater {
    Show-Versions
    Test-RustInstallation
    Write-Host "==> Building MTGO Updater..."
    Set-Location mtgoupdater
    cargo build
    Set-Location ..
    Write-Host "=== Done building MTGO Updater ==="
}

function Test-Mtgoupdater {
    Show-Versions
    Test-RustInstallation
    Write-Host "==> Testing MTGO Updater..."
    Set-Location mtgoupdater
	cargo test -- --nocapture
    Set-Location ..
    Write-Host "=== Done testing MTGO Updater ==="
}

function Build-Clean {
    Remove-Item -Path "mtgoparser/build" -Force -Recurse
    Write-Host "mtgoparser cleaned"
    Set-Location mtgoupdater
    cargo clean
    Write-Host "mtgoupdater cleaned"
    Set-Location ..\mtgogetter
    go clean
    Write-Host "mtgogetter cleaned"
    Set-Location -Path $PSScriptRoot
}

# Define ordered targets
$targets = [ordered]@{
    "all"     = { Build-All }
    "test"    = { Test-All }
    "build-integration"     = { Build-AllIntegration }
    "show-versions" = { Show-Versions }
    "clean"   = { Build-Clean }
    "build-mtgogetter" = { Build-Mtgogetter }
    "test-mtgogetter" = { Test-Mtgogetter }
    "build-mtgoparser" = { Build-Mtgoparser }
    "test-mtgoparser" = { Test-Mtgoparser }
    "bench-mtgoparser" = { Test-MtgoparserBenchmark }
    "build-mtgoupdater" = { Build-Mtgoupdater }
    "test-mtgoupdater" = { Test-Mtgoupdater }
}

# Check if the specified target exists, and if not, show a list of available targets
if ($targets.Contains($target)) {
    # Run the specified target
    & $targets[$target]
} elseif ($target -eq "default") {
    & $targets["all"]
} else {
    Write-Host "Available targets:"
    $targets.Keys | ForEach-Object { Write-Host "  $_" }
}

