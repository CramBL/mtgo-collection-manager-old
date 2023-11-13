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

const MTGOGETTER_BIN: &str = if cfg!(target_os = "windows") {
    "mtgogetter.exe"
} else {
    "mtgogetter"
};

const MTGO_PREPROCESSOR_BIN: &str = if cfg!(target_os = "windows") {
    "mtgo_preprocessor.exe"
} else {
    "mtgo_preprocessor"
};

/// Add `.exe` to the end of the binary name if we're building for Windows
const MTGO_PREPROCESSOR_BIN_PATH: &str = if cfg!(target_os = "windows") {
    concat!(
        "../mtgoparser/build/src/mtgo_preprocessor/Release/mtgo_preprocessor",
        ".exe"
    )
} else {
    "../mtgoparser/build/src/mtgo_preprocessor/Release/mtgo_preprocessor"
};

const BUILD_SCRIPT_EXE: &str = if cfg!(target_os = "windows") {
    ".\\wmake.ps1"
} else {
    "make"
};

const RELEASE_BIN_DIR: &str = "target/release/bin";

const BUILD_MTGOGETTER_CMD: &str = "build-mtgogetter";
const BUILD_MTGOPARSER_CMD: &str = "build-mtgoparser-integration";

fn main() {
    util::detect_changes();

    if cfg!(debug_assertions) {
        print_warn!("Debug mode, skipping custom release build steps");
        return;
    }

    let out_dir = env::var_os("OUT_DIR").unwrap();

    // Create the `bin` directory in the release directory if it doesn't exist
    if !Path::new(RELEASE_BIN_DIR).exists() {
        fs::create_dir(RELEASE_BIN_DIR)
            .unwrap_or_else(|e| panic!("Failed to create {RELEASE_BIN_DIR}: {e}"));
    }

    build::build_all(Path::new(&out_dir));

    include_binaries().unwrap_or_else(|e| panic!("Failed to include binaries in build.rs: {e}"));
}

mod util {
    /// Rerun the build script steps if any of these files or directories change
    pub fn detect_changes() {
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
}

/// Write the binaries to the OUT_DIR and create a file `binaries.rs` that contains the binaries as byte arrays.
fn include_binaries() -> io::Result<()> {
    let out_dir = env::var_os("OUT_DIR").unwrap();
    let dest_path = Path::new(&out_dir).join("binaries.rs");

    // Open the file
    let mtgogetter_bin = Path::new(&out_dir).join(MTGOGETTER_BIN);
    let mut file = File::open(mtgogetter_bin)?;

    // Read the file contents into a vector of bytes
    let mut raw_mtgogetter = Vec::new();
    file.read_to_end(raw_mtgogetter.as_mut())?;

    let mtgogetter_size = raw_mtgogetter.len();

    let mtgo_preprocessor_bin = Path::new(&out_dir).join(MTGO_PREPROCESSOR_BIN);
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
        {WRITE_TO_BIN_DIR_FN}
        "#,
        mtgo_preprocessor_bin = Path::new(&out_dir).join(MTGO_PREPROCESSOR_BIN).display(),
        mtgogetter_bin = Path::new(&out_dir).join(MTGOGETTER_BIN).display(),
    );

    // Write the file contents
    fs::write(dest_path, contents)?;

    Ok(())
}

/// Code to write the binaries to the bin directory
///
/// Creates the `bin` directory in the directory where the binary `MTGO GUI` is located.
///
/// Writes the binaries to the `bin` directory if they don't already exist.
const WRITE_TO_BIN_DIR_FN: &str = r#"
#[cfg(not(debug_assertions))]
fn write_binaries_out() -> std::io::Result<()> {
    const MTGOGETTER_BIN: &str = if cfg!(target_os = "windows") {
        "mtgogetter.exe"
    } else {
        "mtgogetter"
    };
    const MTGO_PREPROCESSOR_BIN: &str = if cfg!(target_os = "windows") {
        "mtgo_preprocessor.exe"
    } else {
        "mtgo_preprocessor"
    };

    let mut path = std::env::current_exe()?;
    path.pop();
    path.push("bin");

    if !path.exists() {
        std::fs::create_dir(&path)?;
    }

    let mtgogetter_bin = path.join(MTGOGETTER_BIN);

    if !mtgogetter_bin.exists() {
        std::fs::write(&mtgogetter_bin, MTGOGETTER)?;
    }

    let mtgo_preprocessor_bin = path.join(MTGO_PREPROCESSOR_BIN);

    if !mtgo_preprocessor_bin.exists() {
        std::fs::write(&mtgo_preprocessor_bin, MTGO_PREPROCESSOR)?;
    }

    Ok(())
}
"#;

mod build {
    use crate::{
        BUILD_MTGOGETTER_CMD, BUILD_MTGOPARSER_CMD, BUILD_SCRIPT_EXE, MTGOGETTER_BIN,
        MTGO_PREPROCESSOR_BIN, MTGO_PREPROCESSOR_BIN_PATH,
    };

    use std::{error::Error, fs, path::Path};

    type Result<T> = std::result::Result<T, Box<dyn Error>>;

    /// Build all binaries and copy them to the OUT_DIR
    pub fn build_all(out_dir: &Path) {
        build_mtgogetter(out_dir).unwrap_or_else(|e| {
            panic!("Failed to build MTGO Getter and copy to OUT_DIR: {out_dir:?}: {e}")
        });

        build_mtgo_preprocessor(out_dir).unwrap_or_else(|e| {
            panic!("Failed to build MTGO Preprocessor and copy to OUT_DIR: {out_dir:?}: {e}")
        });
    }

    /// Build MTGO Getter and copy the binary to the OUT_DIR set by cargo
    fn build_mtgogetter(out_dir: &Path) -> Result<()> {
        let cmd_build_mtgogetter = std::process::Command::new("powershell")
            .args([BUILD_SCRIPT_EXE, BUILD_MTGOGETTER_CMD])
            .current_dir("..")
            .status()?;

        assert!(
            cmd_build_mtgogetter.success(),
            "failed to build MTGO Getter"
        );

        // Copy mtgogetter to the OUT_DIR set by cargo
        let rel_mtgogetter_bin_str = format!("../mtgogetter/{MTGOGETTER_BIN}");
        let rel_mtgogetter_bin_path = Path::new(&rel_mtgogetter_bin_str);

        assert!(
            rel_mtgogetter_bin_path.exists(),
            "Build succeeded but mtgogetter was not found at the expected path ../mtgogetter/{MTGOGETTER_BIN}"
        );

        let dest_path = Path::new(&out_dir).join(MTGOGETTER_BIN);
        fs::copy(rel_mtgogetter_bin_path, &dest_path)?;

        print_warn!("Built MTGO Getter and copied the binary to {dest_path:?}");

        Ok(())
    }

    /// Build MTGO Preprocessor and copy the binary to the OUT_DIR set by cargo
    fn build_mtgo_preprocessor(out_dir: &Path) -> Result<()> {
        let cmd_build_mtgo_preprocessor = std::process::Command::new("powershell")
            .args([BUILD_SCRIPT_EXE, BUILD_MTGOPARSER_CMD])
            .current_dir("..")
            .status()?;

        assert!(
            cmd_build_mtgo_preprocessor.success(),
            "failed to build MTGO Preprocessor"
        );

        assert!(
            Path::new(MTGO_PREPROCESSOR_BIN_PATH).exists(),
            "Build succeeded but mtgo_preprocessor.exe was not found at the expected path {MTGO_PREPROCESSOR_BIN_PATH}",
        );

        // Copy MTGO Preprocessor binary to the OUT_DIR set by cargo
        let dest_path = Path::new(&out_dir).join(MTGO_PREPROCESSOR_BIN);
        fs::copy(Path::new(MTGO_PREPROCESSOR_BIN_PATH), &dest_path).unwrap_or_else(|e| {
            panic!("Failed to copy mtgo_preprocessor binary to {dest_path:?}: {e}")
        });

        print_warn!("Built MTGO Preprocessor and copied the binary to {dest_path:?}");

        Ok(())
    }
}
