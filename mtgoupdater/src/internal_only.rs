use crate::{set_mtgogetter_bin, set_mtgoparser_bin, MTGOGETTER_BIN, MTGOPARSER_BIN};

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

pub fn run_mtgo_preprocessor_gui_example() -> Result<std::process::Output, std::io::Error> {
    crate::util::run_with_args(
        crate::mtgoparser_bin(),
        ["run", "--gui-example", "--caller", "mtgoupdater"],
    )
}
