use std::process::Command;

const MTGOGETTER_BIN: &str = if cfg!(windows) {
    "../mtgogetter/mtgogetter.exe"
} else {
    "../mtgogetter/mtgogetter"
};
const MTGOPARSER_BIN: &str = if cfg!(windows) {
    "../mtgoparser/build/src/mtgo_preprocessor/mtgo_preprocesser.exe"
} else {
    "../mtgoparser/build/src/mtgo_preprocessor/Release/mtgo_preprocesser"
};

pub fn download_goatbots_price_history() -> Result<std::process::Output, Box<dyn std::error::Error>>
{
    let go_exec_out = Command::new(MTGOGETTER_BIN)
        .arg("download")
        .arg("goatbots-price-history")
        .output()?;

    Ok(go_exec_out)
}

pub fn download_goatbots_card_definitions(
) -> Result<std::process::Output, Box<dyn std::error::Error>> {
    let go_exec_out = Command::new(MTGOGETTER_BIN)
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
    let go_exec_out = Command::new(MTGOGETTER_BIN).args(custom_args).output()?;

    Ok(go_exec_out)
}

pub fn run_mtgo_preprocessor_example() -> Result<std::process::Output, Box<dyn std::error::Error>> {
    let pre_processor_exec_out = Command::new(MTGOPARSER_BIN)
        .arg("--caller")
        .arg("mtgoupdater")
        .arg("--run-example")
        .output()?;

    Ok(pre_processor_exec_out)
}

pub fn run_mtgo_preprocessor_json_example(
) -> Result<std::process::Output, Box<dyn std::error::Error>> {
    let pre_processor_exec_out = Command::new(MTGOPARSER_BIN)
        .arg("--caller")
        .arg("mtgoupdater")
        .arg("--run-example-json")
        .output()?;

    Ok(pre_processor_exec_out)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[ignore]
    #[test]
    fn test_call_mtgogetter_download_price_history() {
        assert!(
            std::path::Path::new(MTGOGETTER_BIN).exists(),
            "mtgogetter binary does not exist, build mtgogetter before running this test"
        );
        let test_out = download_goatbots_price_history();
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

    #[ignore]
    #[test]
    fn test_call_mtgogetter_download_card_definitions() {
        assert!(
            std::path::Path::new(MTGOGETTER_BIN).exists(),
            "mtgogetter binary does not exist, build mtgogetter before running this test"
        );
        let test_out = download_goatbots_card_definitions();
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
            std::path::Path::new(MTGOPARSER_BIN).exists(),
            "mtgo_preprocessor binary ({MTGOPARSER_BIN}) does not exist, build mtgoparser before running this test"
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

    #[test]
    fn test_mtgogetter_custom_url_download_scryfall_card_json() {
        // From the repository
        let scryfall_card_json_url = "https://raw.githubusercontent.com/CramBL/mtgo-collection-manager/master/test/test-data/mtgogetter-out/scryfall-card.json";
        let cmd_out = download_custom_url(
            scryfall_card_json_url,
            false,
            None, // Goes to stdout
        );
        match cmd_out {
            Ok(output) => {
                println!("Status:\n{status}", status = output.status,);
                let stdout_as_utf8 = String::from_utf8_lossy(&output.stdout);
                println!("stdout:\n{stdout}", stdout = stdout_as_utf8,);
                println!(
                    "stderr:\n{stderr}",
                    stderr = String::from_utf8_lossy(&output.stderr),
                );
                assert!(stdout_as_utf8.contains(r#""mtgo_id": 25527"#));
                assert_eq!(
                    stdout_as_utf8,
                    include_str!("../../test/test-data/mtgogetter-out/scryfall-card.json")
                );
            }
            Err(e) => panic!("Unexpected error: {e}"),
        }
    }
}
