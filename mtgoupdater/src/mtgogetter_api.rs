use std::ffi::OsStr;

use crate::mtgogetter_bin;
use crate::util;

// Convenience functions for calling mtgogetter
fn run_mtgogetter<'a, I>(args: I) -> Result<std::process::Output, std::io::Error>
where
    I: IntoIterator<Item = &'a str>,
{
    // If we're in debug mode initialize the mtgogetter binary path relative to a subdirectory of the project root
    if cfg!(debug_assertions) {
        crate::internal_only::dev_try_init_mtgogetter_bin();
    }
    util::run_with_args(mtgogetter_bin(), args)
}

pub fn mtgogetter_version() -> Result<std::process::Output, std::io::Error> {
    run_mtgogetter(["--version"])
}

pub fn mtgogetter_update_all(save_to_dir: &OsStr) -> Result<std::process::Output, std::io::Error> {
    run_mtgogetter([
        "update",
        "--save-to-dir",
        save_to_dir
            .to_str()
            .unwrap_or_else(|| panic!("{save_to_dir:?} is not valid unicode")),
    ])
}

pub fn download_goatbots_price_history() -> Result<std::process::Output, std::io::Error> {
    run_mtgogetter(["download", "goatbots-price-history"])
}

pub fn download_goatbots_card_definitions() -> Result<std::process::Output, std::io::Error> {
    run_mtgogetter(["download", "goatbots-card-definitions"])
}

pub fn download_custom_url(
    url: &str,
    decompress: bool,
    save_as: Option<&str>,
) -> Result<std::process::Output, std::io::Error> {
    let mut custom_args = vec!["download", "custom", "url-raw", url];
    if decompress {
        custom_args.push("--decompress");
    }
    if let Some(save_as) = save_as {
        custom_args.push("--save-as");
        custom_args.push(save_as);
    }
    run_mtgogetter(custom_args)
}
