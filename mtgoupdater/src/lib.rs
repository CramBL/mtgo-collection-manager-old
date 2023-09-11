#![allow(dead_code)]
use std::process::Command;

use std::sync::OnceLock;

static MTGOGETTER_BIN: OnceLock<String> = OnceLock::new();
static MTGOPARSER_BIN: OnceLock<String> = OnceLock::new();

pub fn set_mtgogetter_bin(bin_path: &str) -> Result<(), String> {
    MTGOGETTER_BIN.set(bin_path.to_string())
}

pub fn set_mtgoparser_bin(bin_path: &str) -> Result<(), String> {
    MTGOPARSER_BIN.set(bin_path.to_string())
}

fn mtgogetter_bin() -> &'static str {
    MTGOGETTER_BIN.get().expect("MTGOGETTER_BIN not set")
}

fn mtgoparser_bin() -> &'static str {
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
            _ = set_mtgogetter_bin(DEV_MTGOPARSER_BIN);
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

pub fn download_goatbots_price_history() -> Result<std::process::Output, Box<dyn std::error::Error>>
{
    let go_exec_out = Command::new(mtgogetter_bin())
        .arg("download")
        .arg("goatbots-price-history")
        .output()?;

    Ok(go_exec_out)
}

pub fn download_goatbots_card_definitions(
) -> Result<std::process::Output, Box<dyn std::error::Error>> {
    let go_exec_out = Command::new(mtgogetter_bin())
        .arg("download")
        .arg("goatbots-card-definitions")
        .output()?;

    Ok(go_exec_out)
}

pub fn download_custom_url(
    url: &str,
    decompress: bool,
    save_as: Option<&str>,
) -> Result<std::process::Output, Box<dyn std::error::Error>> {
    let mut custom_args = vec!["download", "custom", "url-raw", url];
    if decompress {
        custom_args.push("--decompress");
    }
    if let Some(save_as) = save_as {
        custom_args.push("--save-as");
        custom_args.push(save_as);
    }
    let go_exec_out = Command::new(mtgogetter_bin()).args(custom_args).output()?;

    Ok(go_exec_out)
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

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_call_mtgo_preprocessor() {
        // Check the build directory exists
        assert!(
            std::path::Path::new("../mtgoparser/build").exists(),
            "Build directory does not exist, build mtgoparser before running this test"
        );
        // Check the build src mtgo_preprocessor directory exists
        assert!(
            std::path::Path::new("../mtgoparser/build/src/mtgo_preprocessor").exists(),
            "mtgo_preprocessor directory does not exist, build mtgoparser before running this test"
        );
        // Check the mtgo_preprocessor binary exists
        assert!(
            std::path::Path::new(mtgoparser_bin()).exists(),
            "mtgo_preprocessor binary ({mtgoparser_bin}) does not exist, build mtgoparser before running this test", mtgoparser_bin = mtgoparser_bin()
        );

        let test_out = run_mtgo_preprocessor_example();
        match test_out {
            Ok(output) => {
                println!("Status:\n{status}", status = output.status,);

                println!(
                    "stdout:\n{stdout}",
                    stdout = String::from_utf8_lossy(&output.stdout),
                );
                println!(
                    "stderr:\n{stderr}",
                    stderr = String::from_utf8_lossy(&output.stderr),
                );
            }
            Err(e) => panic!("Unexpected error: {e}"),
        }
    }

    #[test]
    fn test_call_mtgo_preprocessor_json_example() {
        match run_mtgo_preprocessor_json_example() {
            Ok(output) => {
                println!("Status:\n{status}", status = output.status,);

                println!(
                    "stdout:\n{stdout}",
                    stdout = String::from_utf8_lossy(&output.stdout),
                );
                println!(
                    "stderr:\n{stderr}",
                    stderr = String::from_utf8_lossy(&output.stderr),
                );
            }
            Err(e) => panic!("Unexpected error: {e}"),
        }
    }
}
