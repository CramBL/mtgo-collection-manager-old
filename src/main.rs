extern crate pretty_env_logger;
#[macro_use]
extern crate log;

use mtgo_collection_manager::download;
use reqwest::Client;
use serde::{Deserialize, Serialize};
use std::{collections, env, fs, io};

// TODO:

#[derive(Debug, Serialize, Deserialize)]
struct NameQuantityPrices {
    name: String,
    quantity: u32,
    prices: (f32, Option<f32>),
}
impl NameQuantityPrices {
    pub fn new(name: String, quantity: u32, prices: (f32, Option<f32>)) -> Self {
        NameQuantityPrices {
            name,
            quantity,
            prices,
        }
    }
}

#[tokio::main]
async fn main() -> io::Result<()> {
    pretty_env_logger::init();
    info!("Starting price list manager!");
    // Create all directories if they don't exist
    let managed_dir_path = env::current_dir()?.join(mtgo_collection_manager::MANAGED_DIR);
    let prices_dir = managed_dir_path.join("prices");
    let collection_price_history_dir = managed_dir_path.join("collection-price-history");
    fs::create_dir_all(&managed_dir_path)?;
    fs::create_dir_all(&prices_dir)?;
    fs::create_dir_all(&collection_price_history_dir)?;
    let price_list_map: collections::HashMap<String, (f32, Option<f32>)> =
        match download::lib::first_file_match_from_dir(
            mtgo_collection_manager::GOATBOTS_PRICE_LIST_FNAME,
            &prices_dir,
            Some(86400), // 24 hours
        ) {
            // Download and store Goatbots & Cardhoarder price list if there's no file in the managed directory
            None => {
                let client = Client::new();
                // Download Cardhoarder price list (it's big: ~100 MB)
                let cardhoarder_client = client.clone();
                let cardhoader_resp: tokio::task::JoinHandle<serde_json::Value> =
                    tokio::task::spawn(async move {
                        let resp = cardhoarder_client
                            .get(mtgo_collection_manager::CARD_HOARDER_BULK_DATA_URL)
                            .send()
                            .await
                            .expect("Request for cardhoarder bulk data download failed")
                            .text()
                            .await
                            .expect("Failed to get response text");

                        serde_json::from_str(resp.as_str()).expect("JSON was not well-formatted")
                    });
                // Download Goatbots price list
                let resp_bytes = client
                    .get(mtgo_collection_manager::DOWNLOAD_GOATBOTS_PRICE_LIST_URL)
                    .send()
                    .await
                    .unwrap()
                    .bytes()
                    .await
                    .unwrap();
                let readable_bytes = std::io::Cursor::new(resp_bytes);

                // Unzip response and deserialize response
                let gb_contents = download::lib::unzip_bytes(readable_bytes)?;

                // Deserialize Goatbots price list into map
                let gb_id_prices_map =
                    serde_json::from_str::<collections::HashMap<String, f32>>(&gb_contents)
                        .expect("JSON was not well-formatted");

                // Allocate map for map with both prices, and for intermediate CardHoarder price list map
                let mut gb_ch_id_prices_map: collections::HashMap<String, (f32, Option<f32>)> =
                    collections::HashMap::with_capacity(80000);
                let mut ch_id_prices_map: collections::HashMap<String, Option<f32>> =
                    collections::HashMap::with_capacity(30000);

                // Wait for the CardHoarder price list to be downloaded
                let ch_contents: serde_json::Value =
                    cardhoader_resp.await.expect("Cardhoarder request failed");
                ch_contents
                    .as_array()
                    .expect("Cardhoarder response was not an array")
                    // Iterate through CardHoarder price list and insert into intermediate map
                    .iter()
                    .for_each(|e| {
                        if let Some(id) = e["mtgo_id"].as_u64() {
                            let json_id = &id.to_string();
                            if let Some(price) = e["prices"]["tix"].as_str() {
                                let price: f32 = price.parse::<f32>().unwrap();
                                ch_id_prices_map.insert(json_id.to_owned(), Some(price));
                            } else if e["legalities"]["vintage"]
                                .as_str()
                                .unwrap()
                                // 'restricted' or 'legal'
                                .starts_with(|c| c == 'r' || c == 'l')
                            {
                                // Card but sold out at CardHoarder -> price is Null
                                ch_id_prices_map.insert(json_id.to_owned(), None);
                            }
                            // Else: Not a card, do nothing
                        }
                    });
                // Put both price lists into single map
                gb_id_prices_map.into_iter().for_each(|(name, price)| {
                    let ch_price = if let Some(ch_price) = ch_id_prices_map.get(&name) {
                        ch_price.to_owned()
                    } else {
                        None
                    };
                    gb_ch_id_prices_map.insert(name, (price, ch_price));
                });
                // Serialize aggregated map and write to disk
                let contents = serde_json::to_string(&gb_ch_id_prices_map)?;
                download::lib::store_contents(
                    contents,
                    mtgo_collection_manager::GOATBOTS_PRICE_LIST_FNAME,
                    mtgo_collection_manager::MANAGED_PRICE_HISTORY,
                )?;
                gb_ch_id_prices_map
            }
            Some(price_list) => {
                let json_str = fs::read_to_string(price_list)?;
                serde_json::from_str::<collections::HashMap<String, (f32, Option<f32>)>>(&json_str)?
            }
        };

    let card_definitions = match download::lib::first_file_match_from_dir(
        mtgo_collection_manager::CARD_DEFINITIONS_FNAME,
        &managed_dir_path,
        Some(86400), // 24 hours
    ) {
        // Download and store card definitions if there's no file in the managed directory
        None => {
            let response = download::lib::async_get_bytes_readable(
                mtgo_collection_manager::DOWNLOAD_CARD_DEFINITIONS_URL,
            );

            let bytes = match response.await {
                Ok(bytes) => bytes,
                Err(e) => {
                    return Err(io::Error::new(
                        io::ErrorKind::Other,
                        format!("Failed to download card definitions: {e}"),
                    ))
                }
            };

            let contents = download::lib::unzip_bytes(bytes)?;
            download::lib::store_contents(
                contents.clone(),
                mtgo_collection_manager::CARD_DEFINITIONS_FNAME,
                mtgo_collection_manager::MANAGED_DIR,
            )?;
            contents
        }
        Some(card_defs) => fs::read_to_string(card_defs)?,
    };

    let collection_map: collections::HashMap<String, (String, u32)> = if let Some(collection) =
        download::lib::first_file_match_from_dir("collection.json", &managed_dir_path, None)
    {
        let collection = fs::read_to_string(collection)?;
        serde_json::from_str(&collection)?
    } else {
        log::info!(
            "No collection.json found in managed directory, looking for Full Trade List instead"
        );
        if let Some(full_trade_list) =
            download::lib::first_file_match_from_dir("Full Trade List", &managed_dir_path, None)
        {
            let collection = fs::read_to_string(full_trade_list)?;
            let collection_map = mtgo_collection_manager::collection_map_from_string(collection);
            // Write to file
            mtgo_collection_manager::map_to_json_file(
                managed_dir_path,
                "collection.json",
                &collection_map,
            )?;
            collection_map
        } else {
            return Err(io::Error::new(
                io::ErrorKind::Other,
                "Failed to find Full Trade List in managed directory".to_string(),
            ));
        }
    };

    // No use for this yet
    let _card_defs_json: serde_json::Value =
        serde_json::from_str(&card_definitions).expect("JSON was not well-formatted");

    let collection_price_history_map: collections::HashMap<String, NameQuantityPrices> =
        match download::lib::first_file_match_from_dir(
            "collection-price-history-json",
            &collection_price_history_dir,
            Some(86400),
        ) {
            None => {
                // Create new collection price history map from collection map and price list
                let mut collection_price_history: collections::HashMap<String, NameQuantityPrices> =
                    collections::HashMap::with_capacity(75000);
                for (id, (name, quantity)) in collection_map {
                    if id == "1" {
                        // This is an event ticket, insert with price 1.
                        collection_price_history.insert(
                            id,
                            NameQuantityPrices {
                                name,
                                quantity,
                                prices: (1.0, Some(1.0)),
                            },
                        );
                        continue;
                    }
                    let (goatbots_price, cardhoarder_price) =
                        price_list_map.get(&id).unwrap_or_else(|| {
                            panic!(
                                "ID {id} did not yield a result, got: {:#?}",
                                price_list_map[id.as_str()]
                            )
                        });
                    collection_price_history.insert(
                        id,
                        NameQuantityPrices {
                            name,
                            quantity,
                            prices: (*goatbots_price, *cardhoarder_price),
                        },
                    );
                }
                // Store it
                let collection_price_history_json =
                    serde_json::to_string(&collection_price_history)?;
                download::lib::store_contents(
                    collection_price_history_json,
                    "collection-price-history-json",
                    "managed-files\\collection-price-history\\",
                )?;
                collection_price_history
            }
            Some(collection_price_history) => {
                let collection_price_history = fs::read_to_string(collection_price_history)?;
                serde_json::from_str(&collection_price_history)?
            }
        };

    let total_quantity: u32 = collection_price_history_map
        .values()
        .map(
            |NameQuantityPrices {
                 name: _,
                 quantity,
                 prices: _,
             }| quantity,
        )
        .sum();

    println!(
        "{} unique items in collection",
        collection_price_history_map.len()
    );
    println!("{total_quantity} total quantity");

    // top 10 most expensive cards
    let mut top_10: Vec<NameQuantityPrices> = collection_price_history_map
        .values()
        .map(
            |NameQuantityPrices {
                 name,
                 quantity,
                 prices,
             }| { NameQuantityPrices::new(name.to_string(), *quantity, *prices) },
        )
        .collect();
    top_10.sort_by(|card_a, card_b| {
        // Destructure price field
        let (gb_price_a, ch_price_a) = card_a.prices;
        let (gb_price_b, ch_price_b) = card_b.prices;
        // If cardhoarder price is missing, set it to 0.0
        let ch_price_a = ch_price_a.unwrap_or(0.0);
        let ch_price_b = ch_price_b.unwrap_or(0.0);
        // Compare the max of the two prices for card_b to the max for card_a
        gb_price_b
            .max(ch_price_b)
            .partial_cmp(&gb_price_a.max(ch_price_a))
            .unwrap()
    });
    println!("Top 10 most expensive cards:");
    top_10.iter().take(10).for_each(
        |NameQuantityPrices {
             name,
             quantity,
             prices: (gb_price, ch_price),
         }| {
            println!(
                "{name}: {gb_price}$ / {ch_price}$ - {quantity} pcs.",
                ch_price = ch_price.unwrap_or(0.0)
            )
        },
    );
    // Total value
    let total_value: f32 = collection_price_history_map
        .values()
        .map(
            |NameQuantityPrices {
                 name: _,
                 quantity,
                 prices: (gb_price, ch_price),
             }| { gb_price.max(ch_price.unwrap_or(0.0)) * (*quantity as f32) },
        )
        .sum();
    println!("Total value (with Goatbots sell price): {total_value} $");

    Ok(())
}

#[cfg(test)]
mod tests {

    use super::*;
    use serde_json;

    #[test]
    #[ignore] // Ignore because this actually downloads the file from goatbots.com
    fn test_get_zip_and_unzip() {
        let price_url = "https://www.goatbots.com/download/price-history.zip";

        let res_bytes = reqwest::blocking::get(price_url).unwrap().bytes().unwrap();

        println!("Response bytes length: {:?}", res_bytes.len());

        let mut archive = zip::ZipArchive::new(std::io::Cursor::new(res_bytes)).unwrap();
        let mut file = archive.by_index(0).unwrap();
        let mut contents = String::new();
        std::io::Read::read_to_string(&mut file, &mut contents).unwrap();
        // for line in contents.lines() {
        //     println!("{}", line);
        // }
    }

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

    #[test]
    fn test_json_parse_goatbots_price_history() -> std::io::Result<()> {
        let resp = download::lib::get_bytes_readable(
            mtgo_collection_manager::DOWNLOAD_GOATBOTS_PRICE_LIST_URL,
        );
        let bytes = match resp {
            Ok(bytes) => bytes,
            Err(e) => {
                return Err(io::Error::new(
                    io::ErrorKind::Other,
                    format!("Failed to download price list: {e}"),
                ))
            }
        };
        // Unzip response and deserialize response
        let gb_contents = download::lib::unzip_bytes(bytes)?;
        let id_gb_ch_prices_map: collections::HashMap<String, (f32, Option<f32>)> =
            collections::HashMap::with_capacity(80000);
        let gb_id_prices_map: collections::HashMap<String, f32> =
            collections::HashMap::with_capacity(80000);
        let gb_prices_map = serde_json::from_str::<collections::HashMap<String, f32>>(&gb_contents)
            .expect("JSON was not well-formatted");

        println!("{}", gb_prices_map.len());

        Ok(())
    }
}
