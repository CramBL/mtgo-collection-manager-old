#![allow(dead_code)]

use std::sync::OnceLock;

pub mod internal_only;
pub mod mtgo_card;
pub mod mtgo_preprocessor_api;
pub mod mtgogetter_api;
mod util;

pub use mtgogetter_api::download_custom_url as get_custom_url;
pub use mtgogetter_api::download_goatbots_card_definitions as get_goatbots_card_definitions;
pub use mtgogetter_api::download_goatbots_price_history as get_goatbots_price_history;

pub use mtgo_preprocessor_api::run_mtgo_preprocessor_example;
pub use mtgo_preprocessor_api::run_mtgo_preprocessor_json_example as run_process_json_example;

static MTGOGETTER_BIN: OnceLock<String> = OnceLock::new();
static MTGOPARSER_BIN: OnceLock<String> = OnceLock::new();

pub fn mtgo_updater_version() -> &'static str {
    env!("CARGO_PKG_VERSION")
}

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
