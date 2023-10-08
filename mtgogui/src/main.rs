#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")]
#![allow(unused_imports)]
#![allow(dead_code)]

use std::ffi::OsStr;
use std::path::PathBuf;
use std::sync::OnceLock;

use fltk::enums::{Event, Font, FrameType, Shortcut};
use fltk::image::PngImage;
use fltk::text::TextAttr;
use fltk::window::DoubleWindow;
use fltk::{app, button, enums::Color, prelude::*, window::Window};
use fltk::{prelude::*, *};
use fltk_flex::{Flex, FlexType};
use fltk_grid::Grid;
use fltk_table::{SmartTable, TableOpts};
use fltk_theme::{widget_themes, ThemeType, WidgetTheme};

mod collection;
mod menubar;
mod util;

use collection::view::table;
use collection::view::table::column;
use collection::TableMessage;
use menubar::McmMenuBar;
use mtgoupdater::mtgo_preprocessor_api::run_mtgo_preprocessor_version;
use mtgoupdater::mtgogetter_api::mtgogetter_version;

use crate::util::center;

// Directory that stores all collection data
const APP_DATA_DIR: &str = "appdata";

const MIN_APP_WIDTH: i32 = 400;
const MIN_APP_HEIGHT: i32 = 400;
const DEFAULT_APP_WIDTH: i32 = 1400;
const DEFAULT_APP_HEIGHT: i32 = 800;
const WIDGET_PADDING: i32 = 0;

#[derive(Debug, Clone)]
pub enum Message {
    Quit,
    Example,
    MenuBar(menubar::MbMessage),
    Table(collection::TableMessage),
    GotFullTradeList(Box<str>),
}

impl From<menubar::MbMessage> for Message {
    fn from(mb_msg: menubar::MbMessage) -> Self {
        Message::MenuBar(mb_msg)
    }
}

impl From<collection::TableMessage> for Message {
    fn from(ct_msg: collection::TableMessage) -> Self {
        Message::Table(ct_msg)
    }
}

pub struct MtgoGui {
    app: app::App,
    full_tradelist: Option<PathBuf>,
    rcv: app::Receiver<Message>,
    main_win: window::Window,
    menu: McmMenuBar,
    collection: table::CollectionTable,
}

impl MtgoGui {
    pub fn new() -> Self {
        let app = app::App::default();
        let theme = WidgetTheme::new(ThemeType::Dark);
        theme.apply();

        let (ev_send, ev_rcv) = app::channel();
        let mut main_win: DoubleWindow = Window::default()
            .with_size(DEFAULT_APP_WIDTH, DEFAULT_APP_HEIGHT)
            .center_screen()
            .with_label("MTGO Collection Manager");

        main_win.set_icon(Some(util::get_logo()));
        main_win.make_resizable(true);
        main_win.size_range(MIN_APP_WIDTH, MIN_APP_HEIGHT, 0, 0);
        main_win.set_color(Color::Black);
        let menu = McmMenuBar::new(DEFAULT_APP_WIDTH, 25, &ev_send);

        set_left_col_box(ev_send.clone());
        let collection = collection::view::set_collection_main_box(ev_send.clone());

        main_win.end();
        main_win.show();
        main_win.set_callback(move |_| {
            if app::event() == Event::Close {
                ev_send.send(Message::Quit);
            }
        });
        Self {
            app,
            full_tradelist: None,
            rcv: ev_rcv,
            main_win,
            menu,
            collection,
        }
    }

    fn run(&mut self) {
        while self.app.wait() {
            if let Some(msg) = self.rcv.recv() {
                match msg {
                    Message::Quit => {
                        eprintln!("Quit");
                        self.app.quit();
                    }
                    Message::MenuBar(mb_msg) => self.menu.handle_ev(mb_msg),
                    Message::Example => {
                        let cards: Vec<mtgoupdater::mtgo_card::MtgoCard> =
                            mtgoupdater::internal_only::get_example_card_collection();
                        self.collection.set_cards(cards);
                    }
                    Message::Table(t_m) => {
                        self.collection.handle_ev(t_m);
                        self.app.redraw();
                    }
                    Message::GotFullTradeList(full_trade_list_path) => {
                        // TODO: Some basic verification that we actually got a trade list and not some random non-sense.

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
                        let scryfall_path =
                            util::first_file_match_from_dir("scryfall", appdata_dir, None).unwrap();
                        let card_definitions_path =
                            util::first_file_match_from_dir("card-def", appdata_dir, None).unwrap();
                        let price_history_path =
                            util::first_file_match_from_dir("price-his", appdata_dir, None)
                                .unwrap();
                        // Invoke MTGO preprocessor
                        match mtgoupdater::mtgo_preprocessor_api::run_mtgo_preprocessor_parse_full(
                            scryfall_path.as_os_str(),
                            OsStr::new(full_trade_list_path.as_ref()),
                            card_definitions_path.as_os_str(),
                            price_history_path.as_os_str(),
                        ) {
                            Ok(cards) => {
                                eprintln!("MTGO Preprocessor output: {} cards", cards.len());
                                // Fill the progress bar as appropriate
                                // Give all the data to the collection table
                                self.collection.set_cards(cards);
                            }
                            Err(e) => {
                                eprintln!("MTGO Preprocessor error: {e}");
                            }
                        }
                    }
                }
            }
        }
    }
}

impl Default for MtgoGui {
    fn default() -> Self {
        Self::new()
    }
}

fn set_left_col_box(ev_send: app::Sender<Message>) {
    let mut flx_left_col = Flex::default().with_pos(0, 35).with_size(400, 600).column();
    flx_left_col.set_align(enums::Align::LeftTop);
    let mut btn_example = button::Button::new(0, 0, 100, 25, "Example");
    btn_example.set_callback({
        let ev_send = ev_send;
        move |b| {
            ev_send.send(Message::Example);

            b.set_label("Getting example...");
        }
    });
    flx_left_col.end();
}

fn main() {
    if cfg!(debug_assertions) {
        mtgoupdater::internal_only::dev_try_init_mtgogetter_bin();
        mtgoupdater::internal_only::dev_try_init_mtgoparser_bin();
        Flex::debug(true);
    }
    let mut gui = MtgoGui::default();

    gui.run();
}
