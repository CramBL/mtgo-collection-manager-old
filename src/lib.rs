pub mod download;

pub const DOWNLOAD_GOATBOTS_PRICE_LIST_URL: &str =
    "https://www.goatbots.com/download/price-history.zip";
pub const DOWNLOAD_CARD_DEFINITIONS_URL: &str =
    "https://www.goatbots.com/download/card-definitions.zip";

pub const DOWNLOAD_SCRIPT: &str = "download_zip.py";
pub const MANAGED_DIR: &str = "managed-files\\";
pub const MANAGED_PRICE_HISTORY: &str = "managed-files\\prices\\";
pub const GOATBOTS_PRICE_LIST_FNAME: &str = "price-history";
pub const CARD_DEFINITIONS_FNAME: &str = "card-definitions";
pub const CARD_HOARDER_BULK_DATA_URL: &str =
    "https://data.scryfall.io/default-cards/default-cards-20230415210925.json";

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

#[cfg(test)]
mod tests {

    use std::collections;

    use dirs::download_dir;
    use serde_json::Value;

    use super::*;

    #[test]
    fn test_get_card_hoarder_prices() {
        let managed_dir_path = std::env::current_dir().unwrap().join(crate::MANAGED_DIR);
        // let resp = reqwest::blocking::get(CARD_HOARDER_BULK_DATA_URL)
        //     .unwrap()
        //     .text()
        //     .unwrap();

        let oracle_json =
            download::lib::first_file_match_from_dir("oracle-cards", &managed_dir_path, None)
                .unwrap();

        let oracle_json = std::fs::read_to_string(oracle_json).unwrap();

        println!("resp size: {}", oracle_json.len());

        let json_resp: serde_json::Value = serde_json::from_str(oracle_json.as_str()).unwrap();

        let json_collection =
            download::lib::first_file_match_from_dir("collection.json", &managed_dir_path, None)
                .unwrap();

        let collection = std::fs::read_to_string(json_collection).unwrap();
        let collection_map: collections::HashMap<String, (String, u32)> =
            serde_json::from_str(&collection).unwrap();

        println!("Unique cards in collection {}", collection_map.len());

        // Iterate through "mtgo_id" key find match in collection, if match: index into "prices" and "tix"
        match json_resp {
            Value::Array(arr) => {
                println!("was array: len={}", arr.len());
                arr.iter().for_each(|e| {
                    match e["mtgo_id"].as_u64() {
                        Some(id) => {
                            let json_id = &id.to_string();
                            if collection_map.contains_key(json_id) {
                                let res = e["prices"]["tix"].as_str();
                                if res.is_none() {
                                    // if price is null and object is legal in vintage -> card sold out
                                    if e["legalities"]["vintage"].as_str().unwrap().eq("not_legal") {
                                        // Not a card
                                    } else if e["legalities"]["vintage"].as_str().unwrap().eq("legal") {
                                        // card sold out
                                    } else {
                                        panic!("{}", dbg!(e));
                                    }
                                } else {
                                    let card_hoarder_price: f32 = res.unwrap().parse::<f32>().unwrap();
                                    if card_hoarder_price > 10.0 {
                                        let (name, price) = &collection_map[json_id];
                                        println!("{name}: goatbots: {price} cardhoarder: {card_hoarder_price}");
                                    }
                                }

                            }
                        },
                        None => {
                            debug_assert!(e["prices"]["tix"].is_null(), "{}", dbg!(e));
                        }
                    };
                });
            }
            Value::Null => todo!("handle NULL "),
            Value::Bool(_) => todo!("handle Bool"),
            Value::Number(_) => todo!("handle Number"),
            Value::String(_) => todo!("handle String"),
            Value::Object(_) => todo!("handle object"),
        }
    }
}
