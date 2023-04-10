pub mod download;

pub const DOWNLOAD_PRICE_LIST_URL: &str = "https://www.goatbots.com/download/price-history.zip";
pub const DOWNLOAD_CARD_DEFINITIONS_URL: &str =
    "https://www.goatbots.com/download/card-definitions.zip";

pub const DOWNLOAD_SCRIPT: &str = "download_zip.py";
pub const MANAGED_DIR: &str = "managed-files\\";
pub const MANAGED_PRICE_HISTORY: &str = "managed-files\\prices\\";
pub const PRICE_LIST_FNAME: &str = "price-history";
pub const CARD_DEFINITIONS_FNAME: &str = "card-definitions";

pub fn map_to_json_file(
    managed_dir_path: std::path::PathBuf,
    fname: &str,
    map: &std::collections::HashMap<String, (String, u32)>,
) -> Result<(), std::io::Error> {
    let path_to_collection = managed_dir_path.join(fname);
    let mut output_file = std::fs::OpenOptions::new()
        .create(true)
        .write(true)
        .append(true)
        .open(path_to_collection)?;
    use std::io::prelude::*;
    writeln!(output_file, "{}", serde_json::to_string_pretty(&map)?)?;
    Ok(())
}

pub fn collection_map_from_string(
    collection: String,
) -> std::collections::HashMap<String, (String, u32)> {
    let pattern =
        r#"CatID="(?P<id>[0-9]*)".*Quantity="(?P<quantity>[0-9]*)".*Name="(?P<name>.*?)""#;
    let re: regex::Regex = regex::Regex::new(pattern).expect("Failed to compile regex");
    let collection_map: std::collections::HashMap<String, (String, u32)> = re
        .captures_iter(&collection.as_str())
        .map(|cap| {
            (
                cap.name("id").unwrap().as_str().to_string(),
                (
                    cap.name("name").unwrap().as_str().to_string(),
                    cap.name("quantity")
                        .unwrap()
                        .as_str()
                        .parse::<u32>()
                        .unwrap(),
                ),
            )
        })
        .collect();
    collection_map
}
