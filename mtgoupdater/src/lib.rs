#![allow(dead_code)]
use std::process::Command;
use std::sync::OnceLock;

mod mtgogetter_api;

static MTGOGETTER_BIN: OnceLock<String> = OnceLock::new();
static MTGOPARSER_BIN: OnceLock<String> = OnceLock::new();

pub fn set_mtgogetter_bin(bin_path: &str) -> Result<(), String> {
    MTGOGETTER_BIN.set(bin_path.to_string())
}

pub fn set_mtgoparser_bin(bin_path: &str) -> Result<(), String> {
    MTGOPARSER_BIN.set(bin_path.to_string())
}

pub(crate) fn mtgogetter_bin() -> &'static str {
    MTGOGETTER_BIN.get().expect("MTGOGETTER_BIN not set")
}

pub(crate) fn mtgoparser_bin() -> &'static str {
    MTGOPARSER_BIN.get().expect("MTGOPARSER_BIN not set")
}

pub mod internal_only {
    use super::*;

    // Safe to call multiple times from different threads (for tests)
    pub fn dev_try_init_mtgogetter_bin() {
        if MTGOGETTER_BIN.get().is_none() {
            _ = set_mtgogetter_bin(DEV_MTGOGETTER_BIN);
        }
    }
    // Safe to call multiple times from different threads (for tests)
    pub fn dev_try_init_mtgoparser_bin() {
        if MTGOPARSER_BIN.get().is_none() {
            _ = set_mtgoparser_bin(DEV_MTGOPARSER_BIN);
        }
    }

    // Path to the MTGO Getter binary in the repository
    pub const DEV_MTGOGETTER_BIN: &str = if cfg!(windows) {
        "../mtgogetter/mtgogetter.exe"
    } else {
        "../mtgogetter/mtgogetter"
    };
    pub const DEV_MTGOPARSER_BIN: &str = if cfg!(windows) {
        "../mtgoparser/build/src/mtgo_preprocessor/Release/mtgo_preprocesser.exe"
    } else {
        "../mtgoparser/build/src/mtgo_preprocessor/Release/mtgo_preprocesser"
    };
}

pub fn run_mtgo_preprocessor_example() -> Result<std::process::Output, Box<dyn std::error::Error>> {
    let pre_processor_exec_out = Command::new(mtgoparser_bin())
        .arg("--caller")
        .arg("mtgoupdater")
        .arg("--run-example")
        .output()?;

    Ok(pre_processor_exec_out)
}

pub fn run_mtgo_preprocessor_json_example(
) -> Result<std::process::Output, Box<dyn std::error::Error>> {
    let pre_processor_exec_out = Command::new(mtgoparser_bin())
        .arg("--caller")
        .arg("mtgoupdater")
        .arg("--run-example-json")
        .output()?;

    Ok(pre_processor_exec_out)
}
