#![allow(dead_code)]
use std::process::Command;
use std::sync::OnceLock;

pub mod internal_only;
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
