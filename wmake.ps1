param (
    [string]$target = "default",
    [string]$MTGOPARSER_IPO = "On",
    [string]$MTGOPARSER_GENERATOR = "Ninja Multi-Config",
    [string]$BUILD_MODE = "Debug",
    [string]$PACKAGE_NAME = "mtgo-collection-manager"
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
        Write-Error "Error: Go is not installed. Please install Go and try again."
        exit 1
    }
}

function Test-RustInstallation {
    if ($RUST_VERSION -eq "NOT FOUND") {
        Write-Error "Error: Rust is not installed. Please install Rust and try again."
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
    Build-Mtgogui
    Write-Host "================================= "
    Write-Host "=== Done building all targets === "
    Write-Host "================================= "
}

# For integration testing, disabling warnings as errors for mtgoparser
function Build-AllIntegration {
    Build-Mtgogetter
    Build-MtgoparserIntegration
    Build-Mtgoupdater
    Build-Mtgogui
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
        Write-Error "!!! ERROR while setting up build files for MTGO Parser: code ${LASTEXITCODE}"
    } else {
        cmake --build build --config ${MTGOPARSER_BUILD_MODE}
        if ($LASTEXITCODE -ne 0) {
            Write-Error "!!! ERROR while building MTGO Parser: code ${LASTEXITCODE}"
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
    cmake -S . -B build -G "${MTGOPARSER_GENERATOR}" -Dmtgoparser_DEPLOYING_BINARY=On -Dmtgoparser_ENABLE_IPO="${MTGOPARSER_IPO}" -DCMAKE_BUILD_TYPE:STRING=${MTGOPARSER_BUILD_MODE} -Dmtgoparser_ENABLE_COVERAGE:BOOL=${MTGOPARSER_ENABLE_COV} -Dmtgoparser_WARNINGS_AS_ERRORS:BOOL=OFF -Dmtgoparser_ENABLE_CLANG_TIDY:BOOL=OFF -Dmtgoparser_ENABLE_CPPCHECK:BOOL=OFF
    cmake --build build --config ${MTGOPARSER_BUILD_MODE}
    Set-Location ..
    Write-Host "=== Done building MTGO Parser ==="
}

function Test-Mtgoparser {
    Show-Versions
    Write-Host "==> Testing MTGO Parser..."
    Set-Location mtgoparser\build
    ctest --output-on-failure

    if (${LASTEXITCODE} -ne 0) {
        Write-Error "MTGO Parser test failed!"
        Write-Host "Rerunning each test suite with more details until one fails"
        Set-Location test

        $testSuites = @(
            "test_json_parse.exe",
            "test_xml_parse.exe",
            "test_full_collection_parse.exe",
            "tests.exe"
        )

        foreach ($testSuite in $testSuites) {
            & .\Release\$testSuite
            if (${LASTEXITCODE} -ne 0) {
                Write-Error "!!! $testSuite failed"
                break
            }
        }
    }

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
    if ($BUILD_MODE -icontains "release") {
        cargo build -r
    } else {
        cargo build
    }
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

function Build-Mtgogui {
    Show-Versions
    Test-RustInstallation
    Write-Host "==> Building MTGO GUI..."
    Set-Location mtgogui
    if ($BUILD_MODE -icontains "release") {
        cargo build -r
    } else {
        cargo build
    }
    Set-Location ..
    Write-Host "=== Done testing MTGO GUI ==="
}

function Test-Mtgogui {
    Show-Versions
    Test-RustInstallation
    Write-Host "==> Building MTGO GUI..."
    Set-Location mtgogui
    cargo test -- --nocapture
    Set-Location ..
    Write-Host "=== Done testing MTGO GUI ==="
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
    Set-Location ..\mtgogui
    cargo clean
    Set-Location -Path $PSScriptRoot
}

function Build-AndPack {
    Write-Host "-------------------------------------"
    Write-Host "==> Building and packing all binaries"
    Write-Host "-------------------------------------"
    $BUILD_MODE = "Release"
    Build-AllIntegration
    Compress-MCM
    Write-Host "============================================== "
    Write-Host "=== Done building and packing all binaries === "
    Write-Host "===>   ${PACKAGE_NAME}.zip"
    Write-Host "============================================== "
}

function Compress-MCM {
    New-Item -Path .\mtgo-collection-manager -ItemType Directory
    New-Item -Path .\mtgo-collection-manager\bin -ItemType Directory
    Copy-Item -Path .\mtgogetter\mtgogetter.exe -Destination .\mtgo-collection-manager\bin
    # Pack and extract mtgo_preprocessor files
    Set-Location mtgoparser\build
    cpack -G ZIP
    $packedZip = Get-ChildItem -Path $folderPath -Filter "mtgoparser-*.zip" | Select-Object -First 1
    $tempFolder = Join-Path -Path $env:TEMP -ChildPath "ExtractedZip"
    Expand-Archive -Path $packedZip.FullName -DestinationPath $tempFolder -Force
    # Find the first folder within the extracted content
    $firstFolder = Get-ChildItem -Path $tempFolder | Where-Object { $_.PSIsContainer } | Select-Object -First 1
    # Define the source folder within the extracted content
    $sourceFolder = Join-Path -Path $firstFolder.FullName -ChildPath "bin"
    # Copy the contents of the "bin" folder to the destination folder where we'll make the final zip
    Copy-Item -Path $sourceFolder\* -Destination ..\..\mtgo-collection-manager\bin -Recurse -Force
    # Clean up the temporary extraction folder
    Remove-Item -Path $tempFolder -Recurse -Force
    # Back to root
    Set-Location ..\..
    # Copy and rename
    Copy-Item -Path .\mtgogui\target\release\mtgogui.exe -Destination .\mtgo-collection-manager
    Rename-Item -Path .\mtgo-collection-manager\mtgogui.exe -NewName mtgo-collection-manager.exe
    # Make final archive
    Compress-Archive -Path .\mtgo-collection-manager -DestinationPath ".\${PACKAGE_NAME}.zip"
    # Cleanup
    Remove-Item -Path .\mtgo-collection-manager -Recurse
}

# Define ordered targets
$targets = [ordered]@{
    "all"               = { Build-All }
    "test"              = { Test-All }
    "build-integration" = { Build-AllIntegration }
    "show-versions"     = { Show-Versions }
    "clean"             = { Build-Clean }
    "build-mtgogetter"  = { Build-Mtgogetter }
    "test-mtgogetter"   = { Test-Mtgogetter }
    "build-mtgoparser"  = { Build-Mtgoparser }
    "build-mtgoparser-integration" = { Build-MtgoparserIntegration }
    "test-mtgoparser"   = { Test-Mtgoparser }
    "bench-mtgoparser"  = { Test-MtgoparserBenchmark }
    "build-mtgoupdater" = { Build-Mtgoupdater }
    "test-mtgoupdater"  = { Test-Mtgoupdater }
    "build-mtgogui"     = { Build-Mtgogui }
    "test-mtgogui"      = { Test-Mtgogui }
    "pack"              = { Build-AndPack }
    "zip-bins"          = { Compress-MCM }
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

