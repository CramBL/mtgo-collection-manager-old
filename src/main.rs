use chrono;
use std::{path::PathBuf, process::Command};
extern crate pretty_env_logger;
#[macro_use]
extern crate log;

const DOWNLOAD_SCRIPT: &str = "download_zip.py";

const MANAGED_DIR: &str = "managed-files\\";
const MANAGED_PRICE_HISTORY: &str = "managed-files\\prices\\";
const PRICE_LIST_FNAME: &str = "price-history";
const CARD_DEFINITIONS_FNAME: &str = "card-definitions";

fn main() -> std::io::Result<()> {
    pretty_env_logger::init();
    trace!("Starting price list manager!");

    let managed_dir_path = std::env::current_dir().unwrap().join(MANAGED_DIR);
    let download_dir_path = dirs::download_dir().unwrap();

    run_download_script();

    let download_zip_prices =
        list_file_match_from_dir(PRICE_LIST_FNAME, &download_dir_path, Some(600));

    if let Some(zip_prices) = download_zip_prices {
        extract_and_store(zip_prices, PRICE_LIST_FNAME, MANAGED_PRICE_HISTORY);
    }

    if let None = list_file_match_from_dir(CARD_DEFINITIONS_FNAME, &managed_dir_path, None) {
        let download_zip_card_definitions =
            list_file_match_from_dir(CARD_DEFINITIONS_FNAME, &download_dir_path, None);
        if let Some(zip_card_definitions) = download_zip_card_definitions {
            extract_and_store(zip_card_definitions, CARD_DEFINITIONS_FNAME, MANAGED_DIR);
        } else {
            return Err(std::io::Error::new(
                std::io::ErrorKind::Other,
                "No card definitions zip-file in Downloads directory, after attempting to download",
            ));
        }
    }

    if let Some(card_defs) =
        list_file_match_from_dir(CARD_DEFINITIONS_FNAME, &managed_dir_path, None)
    {
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

fn run_download_script() {
    let mut path = std::env::current_dir().unwrap();
    path.push(DOWNLOAD_SCRIPT);
    let output = if cfg!(target_os = "windows") {
        Command::new("cmd")
            .arg("/C")
            .arg("python3.10.exe ".to_string() + path.to_str().unwrap())
            .output()
            .expect("failed to execute process")
    } else {
        Command::new("sh")
            .arg("-c")
            .arg("python3.10 ".to_string() + path.to_str().unwrap())
            .output()
            .expect("failed to execute process")
    };

    let terminal_response = output.stdout;
    info!(
        "Response from command: {}",
        String::from_utf8(terminal_response).unwrap()
    );
}

fn list_file_match_from_dir(
    f_name: &str,
    path: &PathBuf,
    max_file_age: Option<u64>,
) -> Option<std::path::PathBuf> {
    let mut target_lists: Vec<std::path::PathBuf> = Vec::new();

    for entry in path.read_dir().unwrap() {
        let dir_entry = entry.unwrap();

        let metadata = std::fs::metadata(&dir_entry.path()).unwrap();
        let last_modified = metadata.modified().unwrap().elapsed().unwrap().as_secs();
        if metadata.is_file() {
            if let Some(max_file_age) = max_file_age {
                if last_modified > max_file_age {
                    continue;
                }
            }
            debug!(
                "Name: {}, Path:{}",
                dir_entry.path().file_name().unwrap().to_str().unwrap(),
                dir_entry.path().display()
            );

            if dir_entry
                .file_name()
                .to_owned()
                .to_str()
                .unwrap()
                .contains(f_name)
            {
                target_lists.push(dir_entry.path());
            }
        }
    }
    if target_lists.len() > 0 {
        return Some(target_lists[0].clone());
    } else {
        warn!("No target list found");
        return None;
    }
}

fn extract_and_store(path_to_zip: std::path::PathBuf, f_name: &str, pwd_dst_dir: &str) {
    let file = std::fs::File::open(path_to_zip).unwrap();

    let mut archive = zip::ZipArchive::new(file).unwrap();
    let mut file = archive.by_index(0).unwrap();
    assert!(file.name().contains(f_name));

    let mut contents = String::new();
    file.read_to_string(&mut contents).unwrap();
    let mut path = std::env::current_dir().unwrap();
    path.push(pwd_dst_dir);

    let dt: chrono::DateTime<chrono::Local> = chrono::Local::now();
    let time_str = dt.format("%Y-%m-%dT%H-%M").to_string();
    path.push(f_name.to_string() + "-" + &time_str + ".txt");

    let mut output_file = std::fs::OpenOptions::new()
        .create(true)
        .write(true)
        .append(true)
        .open(path)
        .unwrap();
    debug!("Contents: {}", contents[0..100].to_string());
    use std::io::prelude::*;

    for line in contents.lines() {
        writeln!(output_file, "{}", line).unwrap();
    }
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
    fn test_list_file_match_from_dir() {
        let price_res =
            list_file_match_from_dir(PRICE_LIST_FNAME, &dirs::download_dir().unwrap(), Some(1000));
        let card_res = list_file_match_from_dir(
            CARD_DEFINITIONS_FNAME,
            &dirs::download_dir().unwrap(),
            Some(1000),
        );
        println!("Price list: {:?}", price_res);
        println!("Card defs list: {:?}", card_res);
    }

    #[test]
    fn test_extract_and_store() {
        let path = dirs::download_dir().unwrap();
        let path = path.join("price-history (4).zip");
        println!("Path is {}", path.display());
        assert!(path.exists());
        extract_and_store(path, "price-history", "prices");
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
