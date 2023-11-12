use std::fs::File;
use std::io::{self, Read};
use std::path::Path;
use std::{env, fs};

// Convenience macro for printing warnings during the build process
//
// Currently the only way to print "info" messages during the build process (see: https://github.com/rust-lang/cargo/issues/985)
macro_rules! print_warn {
    ($($tokens: tt)*) => {
        println!("cargo:warning={}", format!($($tokens)*))
    }
}

const MTGO_PREPROCESSOR_BIN_PATH_WINDOWS: &str =
    r"..\mtgoparser\build\src\mtgo_preprocessor\Release\mtgo_preprocessor.exe";

const RELEASE_BIN_PATH: &str = "target/release/bin";

fn main() {
    detect_changes();

    if cfg!(debug_assertions) {
        print_warn!("Debug mode, skipping custom release build steps");
        return;
    }

    let out_dir = env::var_os("OUT_DIR").unwrap();

    // Create the `bin` directory in the release directory if it doesn't exist
    if !Path::new(RELEASE_BIN_PATH).exists() {
        fs::create_dir(RELEASE_BIN_PATH)
            .unwrap_or_else(|_| panic!("Failed to create {RELEASE_BIN_PATH}"));
    }

    if cfg!(target_os = "windows") {
        print_warn!("Building for windows");
        // Build MTGO Getter
        let cmd_go_getter = std::process::Command::new("powershell")
            .args([".\\wmake.ps1", "build-mtgogetter"])
            .current_dir("..")
            .status();
        assert!(cmd_go_getter.is_ok(), "failed to build MTGO Getter");

        // Copy mtgogetter.exe to the release directory
        fs::copy(
            Path::new("../mtgogetter/mtgogetter.exe"),
            Path::new(RELEASE_BIN_PATH).join("mtgogetter.exe"),
        )
        .unwrap_or_else(|_| panic!("Failed to copy mtgogetter.exe to {RELEASE_BIN_PATH}"));
        assert!(
            Path::new("../mtgogetter/mtgogetter.exe").exists(),
            "Build succeeded but mtgogetter.exe was not found at the expected path ../mtgogetter/mtgogetter.exe"
        );

        let dest_path = Path::new(&out_dir).join("mtgogetter.exe");
        fs::copy(Path::new("../mtgogetter/mtgogetter.exe"), dest_path)
            .unwrap_or_else(|_| panic!("Failed to copy mtgogetter.exe to {RELEASE_BIN_PATH}"));

        print_warn!("Built MTGO Getter and copied mtgogetter.exe to {RELEASE_BIN_PATH}");

        let cmd_ps_make = std::process::Command::new("powershell")
            .args([".\\wmake.ps1", "build-mtgoparser-integration"])
            .current_dir("..")
            .status();
        assert!(
            cmd_ps_make.is_ok(),
            "failed to build MTGO Parser/Preprocessor"
        );

        // Copy the produced binary to the release directory
        copy_all_from_dir(
            Path::new(MTGO_PREPROCESSOR_BIN_PATH_WINDOWS)
                .parent()
                .unwrap(),
            Path::new(RELEASE_BIN_PATH),
        )
        .unwrap_or_else(|_| {
            panic!("Failed to copy mtgo_preprocessor.exe and related files to {RELEASE_BIN_PATH}")
        });
        assert!(
            Path::new(MTGO_PREPROCESSOR_BIN_PATH_WINDOWS).exists(),
            "Build succeeded but mtgo_preprocessor.exe was not found at the expected path {}",
            MTGO_PREPROCESSOR_BIN_PATH_WINDOWS
        );

        let dest_path = Path::new(&out_dir).join("mtgo_preprocessor.exe");
        fs::copy(Path::new(MTGO_PREPROCESSOR_BIN_PATH_WINDOWS), dest_path).unwrap_or_else(|_| {
            panic!("Failed to copy mtgo_preprocessor.exe to {RELEASE_BIN_PATH}")
        });
    }

    if cfg!(target_os = "linux") {
        eprintln!("linux");
        let cmd_make = std::process::Command::new("make build-mtgoparser-integration")
            .current_dir("..")
            .status()
            .expect("failed to execute process");
        assert!(cmd_make.success());
    }

    include_binaries().unwrap_or_else(|_| panic!("Failed to include binaries in build.rs"));
}

// Copy all files from the source directory to the destination directory
fn copy_all_from_dir(src: &Path, dest: &Path) -> io::Result<()> {
    // Iterate over the entries in the source directory
    for entry in fs::read_dir(src)? {
        let entry = entry?;
        let source_file = entry.path();

        // Create the destination path by appending the file name to the destination directory
        let dest_file = dest.join(entry.file_name());

        // Copy the file
        fs::copy(&source_file, &dest_file)?;

        print_warn!(
            "Copied: {} to {}",
            source_file.display(),
            dest_file.display()
        );
    }

    Ok(())
}

/// Rerun if any of these files or directories change
fn detect_changes() {
    // Build script itself
    println!("cargo:rerun-if-changed=build.rs");
    // MTGO Parser files (source code)
    println!("cargo:rerun-if-changed=../mtgoparser/src/mtgo_preprocessor");
    println!("cargo:rerun-if-changed=../mtgoparser/include");
    // MTGO Updater
    println!("cargo:rerun-if-changed=../mtgoupdater/src");
    // MTGO Getter
    println!("cargo:rerun-if-changed=../mtgogetter/main.go");
    println!("cargo:rerun-if-changed=../mtgogetter/pkg");
    println!("cargo:rerun-if-changed=../mtgogetter/cmd");
}

fn include_binaries() -> io::Result<()> {
    let out_dir = env::var_os("OUT_DIR").unwrap();
    let dest_path = Path::new(&out_dir).join("binaries.rs");

    // Open the file
    let mtgogetter_bin = Path::new(RELEASE_BIN_PATH).join("mtgogetter.exe");
    let mut file = File::open(mtgogetter_bin)?;

    // Read the file contents into a vector of bytes
    let mut raw_mtgogetter = Vec::new();
    file.read_to_end(raw_mtgogetter.as_mut())?;

    let mtgogetter_size = raw_mtgogetter.len();

    let mtgo_preprocessor_bin = Path::new(RELEASE_BIN_PATH).join("mtgo_preprocessor.exe");
    let mut file = File::open(mtgo_preprocessor_bin)?;

    // Read the file contents into a vector of bytes
    let mut raw_mtgo_preprocessor = Vec::new();
    file.read_to_end(raw_mtgo_preprocessor.as_mut())?;

    let mtgo_preprocessor_size = raw_mtgo_preprocessor.len();

    // format contents
    let contents = format!(
        r#"
        #[cfg(not(debug_assertions))]
        pub const MTGO_PREPROCESSOR: &[u8; {mtgo_preprocessor_size}] = include_bytes!(r"{mtgo_preprocessor_bin}");
        #[cfg(not(debug_assertions))]
        pub const MTGOGETTER: &[u8; {mtgogetter_size}] = include_bytes!(r"{mtgogetter_bin}");
        "#,
        mtgo_preprocessor_bin = Path::new(&out_dir).join("mtgo_preprocessor.exe").display(),
        mtgogetter_bin = Path::new(&out_dir).join("mtgogetter.exe").display(),
    );

    // Write the file contents
    fs::write(dest_path, contents)?;

    Ok(())
}
