use std::ffi::OsStr;

use crate::{mtgoparser_bin, util};

// Convenience functions for calling mtgoparser
fn run_mtgo_preprocessor<'a, I>(args: I) -> Result<std::process::Output, std::io::Error>
where
    I: IntoIterator<Item = &'a str>,
{
    util::run_with_args(mtgoparser_bin(), args)
}

pub fn run_mtgo_preprocessor_version() -> Result<std::process::Output, std::io::Error> {
    run_mtgo_preprocessor(["--version"])
}

pub fn run_mtgo_preprocessor_parse_full(
    scryfall_path: &OsStr,
    full_trade_list_path: &OsStr,
    card_definitions_path: &OsStr,
    price_history_path: &OsStr,
) -> Result<Vec<crate::mtgo_card::MtgoCard>, std::io::Error> {
    let out = run_mtgo_preprocessor([
        "run",
        "-u",
        "--scryfall-path",
        scryfall_path
            .to_str()
            .expect("scryfall_path is not valid unicode"),
        "--full-trade-list",
        full_trade_list_path
            .to_str()
            .expect("full_trade_list_path is not valid unicode"),
        "--card-definitions",
        card_definitions_path
            .to_str()
            .expect("card_definitions_path is not valid unicode"),
        "--price-history",
        price_history_path
            .to_str()
            .expect("price_history_path is not valid unicode"),
    ])?;
    let stdout_json = String::from_utf8_lossy(&out.stdout);
    Ok(serde_json::from_str(&stdout_json).unwrap())
}
