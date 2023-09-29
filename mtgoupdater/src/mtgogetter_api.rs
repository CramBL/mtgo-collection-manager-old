use crate::mtgogetter_bin;
use crate::util;

// Convenience functions for calling mtgogetter
fn run_mtgogetter<'a, I>(args: I) -> Result<std::process::Output, std::io::Error>
where
    I: IntoIterator<Item = &'a str>,
{
    util::run_with_args(mtgogetter_bin(), args)
}

pub fn mtgogetter_version() -> Result<std::process::Output, std::io::Error> {
    run_mtgogetter(["--version"])
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
