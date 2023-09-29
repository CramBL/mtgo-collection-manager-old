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

pub fn run_mtgo_preprocessor_example() -> Result<std::process::Output, std::io::Error> {
    run_mtgo_preprocessor([
        "run",
        "--example-json-formats",
        "--example-scryfall",
        "--caller",
        "mtgoupdater",
    ])
}

pub fn run_mtgo_preprocessor_json_example() -> Result<std::process::Output, std::io::Error> {
    run_mtgo_preprocessor(["run", "--example-json-formats", "--caller", "mtgoupdater"])
}
