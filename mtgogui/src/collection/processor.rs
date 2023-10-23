use std::{
    ffi::OsStr,
    io::Error,
    path::{Path, PathBuf},
};

use fltk::app::Sender;

use crate::{
    appdata::{paths::AppdataPaths, APP_DATA_DIR},
    util::first_file_match_from_dir,
    Message,
};

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
                            eprintln!("MTGO Getter error: {e}");
                        }
                    }
                    // TOOD: Fill the progress bar as appropriate
                    // Give the full trade list to the parser
                    // Find all the most recent files in the appdata directory
                    let appdata_paths = match AppdataPaths::new() {
                        Ok(paths) => paths,
                        Err(err) => {
                            eprintln!("{err}");
                            return;
                        }
                    };

                    // Invoke MTGO preprocessor
                    match mtgoupdater::mtgo_preprocessor_api::run_mtgo_preprocessor_parse_full(
                        appdata_paths.scryfall_path(),
                        OsStr::new(full_trade_list_path.as_ref()),
                        appdata_paths.card_definitions_path(),
                        appdata_paths.price_history_path(),
                        Some(appdata_paths.appdata_dir_path()),
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
