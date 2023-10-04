use mtgoupdater::internal_only;
use pretty_assertions::assert_eq;

#[ignore]
#[test]
fn test_call_mtgogetter_download_price_history() {
    internal_only::dev_try_init_mtgogetter_bin();
    let test_out = mtgoupdater::get_goatbots_price_history();
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
    internal_only::dev_try_init_mtgogetter_bin();
    let test_out = mtgoupdater::get_goatbots_card_definitions();
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
fn test_mtgogetter_custom_url_download_scryfall_card_json() {
    internal_only::dev_try_init_mtgogetter_bin();
    // From the repository
    let scryfall_card_json_url = "https://raw.githubusercontent.com/CramBL/mtgo-collection-manager/master/test/test-data/mtgogetter-out/scryfall-card.json";
    let cmd_out = mtgoupdater::get_custom_url(
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
