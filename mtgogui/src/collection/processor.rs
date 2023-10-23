use std::{ffi::OsStr, path::Path};

use fltk::app::Sender;

use crate::{util::first_file_match_from_dir, Message, APP_DATA_DIR};

#[derive(Debug)]
pub struct TradelistProcessor {
    event_sender: Sender<Message>,
}

impl TradelistProcessor {
    pub fn new(ev_send: Sender<Message>) -> Self {
        Self {
            event_sender: ev_send,
        }
    }

    pub fn process(&mut self, full_trade_list_path: Box<Path>) {
        // TODO: Some basic verification that we actually got a trade list and not some random non-sense.
        let trade_list_processor_thread =
            std::thread::Builder::new().name("Trade List Processor".to_string());
        let _handle = trade_list_processor_thread
            .spawn({
                let sender = self.event_sender.clone();
                move || {
                    // Invoke MTGO getter
                    match mtgoupdater::mtgogetter_api::mtgogetter_update_all(OsStr::new(
                        APP_DATA_DIR,
                    )) {
                        Ok(output) => {
                            eprintln!("MTGO Getter output: {}", output.status);
                        }
                        Err(e) => {
                            eprintln!("MTGO Getter error: {}", e);
                        }
                    }
                    // TOOD: Fill the progress bar as appropriate
                    // Give the full trade list to the parser
                    // Find all the most recent files in the appdata directory
                    let appdata_dir = std::path::Path::new(APP_DATA_DIR);
                    if !appdata_dir.exists() {
                        eprintln!("App data path doesn't exist:{APP_DATA_DIR}");
                        return;
                    }
                    let scryfall_path =
                        if let Some(p) = first_file_match_from_dir("scryfall", appdata_dir, None) {
                            p
                        } else {
                            eprintln!("Could not locate Scryfall data json in {APP_DATA_DIR}");
                            return;
                        };

                    let card_definitions_path =
                        if let Some(p) = first_file_match_from_dir("card-def", appdata_dir, None) {
                            p
                        } else {
                            eprintln!("Could not locate card definition json in {APP_DATA_DIR}");
                            return;
                        };
                    let price_history_path = if let Some(p) =
                        first_file_match_from_dir("price-his", appdata_dir, None)
                    {
                        p
                    } else {
                        eprintln!("Could not locate price history json");
                        return;
                    };
                    let appdata_dir_str = format!("{APP_DATA_DIR}/");
                    let appdata_dir_path = OsStr::new(&appdata_dir_str);
                    // Invoke MTGO preprocessor
                    match mtgoupdater::mtgo_preprocessor_api::run_mtgo_preprocessor_parse_full(
                        scryfall_path.as_os_str(),
                        OsStr::new(full_trade_list_path.as_ref()),
                        card_definitions_path.as_os_str(),
                        price_history_path.as_os_str(),
                        Some(appdata_dir_path),
                    ) {
                        Ok(cards) => {
                            eprintln!("MTGO Preprocessor output: {} cards", cards.len());
                            // Fill the progress bar as appropriate
                            // Give all the data to the collection table

                            sender.send(Message::SetCards(cards));
                        }
                        Err(e) => {
                            eprintln!("MTGO Preprocessor error: {e}");
                        }
                    }
                }
            })
            .expect("Failed spawning Trade List Processor thread");
    }
}
