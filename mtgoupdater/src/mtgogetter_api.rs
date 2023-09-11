use std::process::Command;

use crate::mtgogetter_bin;

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
