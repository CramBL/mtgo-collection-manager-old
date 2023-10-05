use crate::{set_mtgogetter_bin, set_mtgoparser_bin, MTGOGETTER_BIN, MTGOPARSER_BIN};

// Safe to call multiple times from different threads (for tests)
pub fn dev_try_init_mtgogetter_bin() {
    if MTGOGETTER_BIN.get().is_none() {
        _ = set_mtgogetter_bin(DEV_MTGOGETTER_BIN.into());
    }
}
// Safe to call multiple times from different threads (for tests)
pub fn dev_try_init_mtgoparser_bin() {
    if MTGOPARSER_BIN.get().is_none() {
        _ = set_mtgoparser_bin(DEV_MTGOPARSER_BIN.into());
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

pub fn run_mtgo_preprocessor_gui_example() -> Result<std::process::Output, std::io::Error> {
    crate::util::run_with_args(
        crate::mtgoparser_bin(),
        ["run", "--gui-example", "--caller", "mtgoupdater"],
    )
}

pub fn run_mtgo_preprocessor_example_collection_json_stdout(
) -> Result<std::process::Output, std::io::Error> {
    crate::util::run_with_args(crate::mtgoparser_bin(), ["run", "--collection-json-out"])
}

pub fn get_example_card_collection() -> Vec<crate::mtgo_card::MtgoCard> {
    let out = run_mtgo_preprocessor_example_collection_json_stdout().unwrap();
    let stdout_json = String::from_utf8_lossy(&out.stdout);
    serde_json::from_str(&stdout_json).unwrap()
}
