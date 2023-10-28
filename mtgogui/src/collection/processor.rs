use std::{
    ffi::OsStr,
    io::Error,
    path::{Path, PathBuf},
    thread,
};

use fltk::app::Sender;

use crate::{
    appdata::{update::AppData, APP_DATA_DIR},
    menubar::util::ProgressUpdate,
    menubar::MenubarMessage,
    util::first_file_match_from_dir,
    Message,
};

/// [TradelistProcessor] is responsible for processing the tradelist, updating the card data, and assigning the cards to the collection table.
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
                    sender.send(Message::MenuBar(MenubarMessage::ProgressBar(
                        ProgressUpdate {
                            show: true,
                            progress: 5.,
                            label: "Processing trade list...".into(),
                        },
                    )));

                    // Invoke MTGO getter
                    sender.send(Message::MenuBar(MenubarMessage::ProgressBar(
                        ProgressUpdate {
                            show: true,
                            progress: 10.,
                            label: "Updating card data...".into(),
                        },
                    )));

                    // Give the full trade list to the parser
                    // Find all the most recent files in the appdata directory, download and update them if necessary
                    let appdata_paths = match AppData::update() {
                        Ok(paths) => paths,
                        Err(err) => {
                            log::info!("{err}");
                            return;
                        }
                    };

                    sender.send(Message::MenuBar(MenubarMessage::ProgressBar(
                        ProgressUpdate {
                            show: true,
                            progress: 70.,
                            label: "Update done".into(),
                        },
                    )));

                    // Invoke MTGO preprocessor
                    log::info!("Running MTGO Preprocessor");
                    log::info!("Scryfall path: {p:?}", p = appdata_paths.scryfall_path());
                    log::info!(
                        "Card definitions path: {p:?}",
                        p = appdata_paths.card_definitions_path()
                    );
                    log::info!(
                        "Price history path: {p:?}",
                        p = appdata_paths.price_history_path()
                    );
                    log::info!("Save to dir: {p:?}", p = appdata_paths.appdata_dir_path());

                    sender.send(Message::MenuBar(MenubarMessage::ProgressBar(
                        ProgressUpdate {
                            show: true,
                            progress: 75.,
                            label: "Processing updated card data...".into(),
                        },
                    )));

                    match mtgoupdater::mtgo_preprocessor_api::run_mtgo_preprocessor_parse_full(
                        appdata_paths.scryfall_path(),
                        OsStr::new(full_trade_list_path.as_ref()),
                        appdata_paths.card_definitions_path(),
                        appdata_paths.price_history_path(),
                        Some(appdata_paths.appdata_dir_path()),
                    ) {
                        Ok(cards) => {
                            log::info!("MTGO Preprocessor output: {} cards", cards.len());
                            // Fill the progress bar as appropriate
                            // Give all the data to the collection table
                            sender.send(Message::MenuBar(MenubarMessage::ProgressBar(
                                ProgressUpdate {
                                    show: true,
                                    progress: 95.,
                                    label: "Processing complete!".into(),
                                },
                            )));

                            complete_progress_bar(sender.clone());

                            sender.send(Message::SetCards(cards));
                        }
                        Err(e) => {
                            log::info!("MTGO Preprocessor error: {e}");
                        }
                    }
                }
            })
            .expect("Failed spawning Trade List Processor thread");
    }
}

/// Spawn a thread to set the progress bar to 100% and then hide it after a second
fn complete_progress_bar(ev_sender: Sender<Message>) {
    thread::spawn({
        move || {
            ev_sender.send(Message::MenuBar(MenubarMessage::ProgressBar(
                ProgressUpdate {
                    show: true,
                    progress: 100.,
                    label: "Updating complete".into(),
                },
            )));
            thread::sleep(std::time::Duration::from_secs(1));
            ev_sender.send(Message::MenuBar(MenubarMessage::ProgressBar(
                ProgressUpdate {
                    show: false,
                    progress: 0.,
                    label: "".into(),
                },
            )));
        }
    });
}
