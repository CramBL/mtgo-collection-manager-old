use chrono;
extern crate pretty_env_logger;
#[macro_use]
extern crate log;

use mtgo_collection_manager::download;

fn main() -> std::io::Result<()> {
    pretty_env_logger::init();
    trace!("Starting price list manager!");

    let managed_dir_path = std::env::current_dir()
        .unwrap()
        .join(mtgo_collection_manager::MANAGED_DIR);
    let download_dir_path = dirs::download_dir().unwrap();

    download::lib::run_download_script(mtgo_collection_manager::DOWNLOAD_SCRIPT);

    let download_zip_prices = download::lib::first_file_match_from_dir(
        mtgo_collection_manager::PRICE_LIST_FNAME,
        &download_dir_path,
        Some(600),
    );

    if let Some(zip_prices) = download_zip_prices {
        download::lib::extract_and_store(
            zip_prices,
            mtgo_collection_manager::PRICE_LIST_FNAME,
            mtgo_collection_manager::MANAGED_PRICE_HISTORY,
        );
    }

    if let None = download::lib::first_file_match_from_dir(
        mtgo_collection_manager::CARD_DEFINITIONS_FNAME,
        &managed_dir_path,
        None,
    ) {
        let download_zip_card_definitions = download::lib::first_file_match_from_dir(
            mtgo_collection_manager::CARD_DEFINITIONS_FNAME,
            &download_dir_path,
            None,
        );
        if let Some(zip_card_definitions) = download_zip_card_definitions {
            download::lib::extract_and_store(
                zip_card_definitions,
                mtgo_collection_manager::CARD_DEFINITIONS_FNAME,
                mtgo_collection_manager::MANAGED_DIR,
            );
        } else {
            return Err(std::io::Error::new(
                std::io::ErrorKind::Other,
                "No card definitions zip-file in Downloads directory, after attempting to download",
            ));
        }
    }

    if let Some(card_defs) = download::lib::first_file_match_from_dir(
        mtgo_collection_manager::CARD_DEFINITIONS_FNAME,
        &managed_dir_path,
        None,
    ) {
        let card_definitions = std::fs::read_to_string(card_defs).unwrap();
        let card_definitions: Vec<String> = card_definitions
            .lines()
            .map(|line| line.to_string())
            .collect();
        let first_10_lines = &card_definitions[0..10];
        info!("Card definitions: {:?}", first_10_lines);
    } else {
        return Err(std::io::Error::new(
            std::io::ErrorKind::Other,
            "No card definitions file in managed-files directory",
        ));
    }

    Ok(())
}

#[cfg(test)]
mod tests {

    use super::*;
    use serde_json;

    #[test]
    fn test_card_defs() {
        let card_definitions = std::fs::read_to_string("card-definitions.txt").unwrap();
        let prices = std::fs::read_to_string("prices\\price-history-2023-04-03T21-10.txt").unwrap();
        let prices_json: serde_json::Value =
            serde_json::from_str(prices.as_str()).expect("JSON was not well-formatted");
        let cards_json: serde_json::Value =
            serde_json::from_str(card_definitions.as_str()).expect("JSON was not well-formatted");
        let card = cards_json["108818"].as_object().unwrap();
        let price = prices_json["108818"].as_f64().unwrap();
        println!("Card definitions: {:?}", card);
        println!(
            "Name: {}\nPrice: {:?}",
            card["name"].as_str().unwrap(),
            price
        );
        let keys_cards = cards_json.as_object().unwrap().keys();
        keys_cards.into_iter().for_each(|key| {
            let card = cards_json[key].as_object().unwrap();
            let price = prices_json[key].as_f64().unwrap();
            println!(
                "Name: {}\nPrice: {:?}",
                card["name"].as_str().unwrap(),
                price
            );
        });
    }

    #[test]
    fn test_time_str() {
        let mut path = std::env::current_dir().unwrap();
        println!("The current directory is {}", path.display());
        path.push("prices\\");
        println!("Path is {}", path.display());
        let dt: chrono::DateTime<chrono::Local> = chrono::Local::now();
        let time_str = dt.format("%Y-%m-%dT%H-%M").to_string();
        println!("Time str is {}", time_str);
        let filename = format!("price-history-{time_str}.txt");
        println!("Filename is {}", filename);
        path.push(filename.to_string());

        use std::fs::OpenOptions;

        let mut file = OpenOptions::new()
            .read(true)
            .write(true)
            .create(true)
            .open(path)
            .unwrap();
        use std::io::prelude::*;
        // Write a &str in the file (ignoring the result).
        writeln!(&mut file, "Hello World!").unwrap();

        file.flush().unwrap();
    }

    #[test]
    fn test_xml_parse() {
        let mut path = dirs::download_dir().unwrap();
        path.push("Full Trade List.dek");
        let file = std::fs::File::open(path).unwrap();
        let mut file = std::io::BufReader::new(file);

        let mut buf = String::new();
        let _bytes_read = file.read_to_string(&mut buf).unwrap();
        let pattern =
            r#"CatID="(?P<id>[0-9]*).*Quantity="(?P<quantity>[0-9]*).*Name="(?P<name>[\s\w]*)"#;
        let re: regex::Regex = regex::Regex::new(pattern).unwrap();
        let collection_map: std::collections::HashMap<String, (String, u32)> = re
            .captures_iter(buf.as_str())
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

        // Write to file
        let mut path = std::env::current_dir().unwrap();
        path.push("collection.json");
        let mut output_file = std::fs::OpenOptions::new()
            .create(true)
            .write(true)
            .append(true)
            .open(path)
            .unwrap();
        use std::io::prelude::*;
        writeln!(
            output_file,
            "{}",
            serde_json::to_string_pretty(&collection_map).unwrap()
        )
        .unwrap();
    }
}
