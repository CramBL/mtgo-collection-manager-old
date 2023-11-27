#![allow(dead_code)]

use std::ffi::OsStr;
use std::ffi::OsString;
use std::sync::OnceLock;

pub mod internal_only;
pub mod mtgo_card;
pub mod mtgo_preprocessor_api;
pub mod mtgogetter_api;
mod util;
mod zip;

pub use mtgogetter_api::download_custom_url as get_custom_url;
pub use mtgogetter_api::download_goatbots_card_definitions as get_goatbots_card_definitions;
pub use mtgogetter_api::download_goatbots_price_history as get_goatbots_price_history;

static MTGOGETTER_BIN: OnceLock<OsString> = OnceLock::new();
static MTGOPARSER_BIN: OnceLock<OsString> = OnceLock::new();

/// Returns the version of `MTGO Updater`
pub fn mtgo_updater_version() -> &'static str {
    env!("CARGO_PKG_VERSION")
}

/// Sets the path to the `MTGO Getter` binary
///
/// # Arguments
///
/// * `bin_path` - Path to the `MTGO Getter` binary
///
/// # Errors
///
/// Returns an error if path has already been set.
pub fn set_mtgogetter_bin(bin_path: OsString) -> Result<(), OsString> {
    MTGOGETTER_BIN.set(bin_path)
}

/// Sets the path to the binary of `MTGO Parser`/`MTGO Preprocessor`
///
/// # Arguments
///
/// * `bin_path` - Path to the `MTGO Parser`/`MTGO Preprocessor` binary
///
/// # Errors
///
/// Returns an error if path has already been set.
pub fn set_mtgoparser_bin(bin_path: OsString) -> Result<(), OsString> {
    MTGOPARSER_BIN.set(bin_path)
}

/// Gets the path to the `MTGO Getter` binary
///
/// If the path has not been set, it will be set to the default path relative to the current executable
///
/// # Panics
///
/// Panics if the current executable path cannot be determined
pub(crate) fn mtgogetter_bin() -> &'static OsStr {
    MTGOGETTER_BIN.get_or_init(|| {
        let mut path = std::env::current_exe().expect("Failed to get current executable path");
        path.pop();
        path.push("bin");
        path.push("mtgogetter");
        if cfg!(windows) {
            path.set_extension(std::env::consts::EXE_EXTENSION);
        }
        path.into_os_string()
    })
}

/// Gets the path to the `MTGO Parser`/`MTGO Preprocessor` binary
///
/// If the path has not been set, it will be set to the default path relative to the current executable
///
/// # Panics
///
/// Panics if the current executable path cannot be determined
pub(crate) fn mtgoparser_bin() -> &'static OsStr {
    MTGOPARSER_BIN.get_or_init(|| {
        let mut path = std::env::current_exe().expect("Failed to get current executable path");
        path.pop();
        path.push("bin");
        path.push("mtgo_preprocessor");
        if cfg!(windows) {
            path.set_extension(std::env::consts::EXE_EXTENSION);
        }
        path.into_os_string()
    })
}
