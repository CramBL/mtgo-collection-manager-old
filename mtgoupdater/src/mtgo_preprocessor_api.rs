use std::process::Command;

use crate::mtgoparser_bin;

pub fn run_mtgo_preprocessor_example() -> Result<std::process::Output, std::io::Error> {
    let pre_processor_exec_out = Command::new(mtgoparser_bin())
        .arg("--caller")
        .arg("mtgoupdater")
        .arg("--example-json")
        .arg("--example-scryfall")
        .arg("run")
        .output()?;

    Ok(pre_processor_exec_out)
}

pub fn run_mtgo_preprocessor_json_example() -> Result<std::process::Output, std::io::Error> {
    let pre_processor_exec_out = Command::new(mtgoparser_bin())
        .arg("run")
        .arg("--example-json-formats")
        .arg("--caller")
        .arg("mtgoupdater")
        .output()?;

    Ok(pre_processor_exec_out)
}

pub fn run_mtgo_preprocessor_version() -> Result<std::process::Output, std::io::Error> {
    let pre_processor_exec_out = Command::new(mtgoparser_bin()).arg("--version").output()?;

    Ok(pre_processor_exec_out)
}
