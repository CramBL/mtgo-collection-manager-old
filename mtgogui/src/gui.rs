use std::ffi::OsStr;
use std::path::PathBuf;
use std::sync::OnceLock;

use crate::appdata::state::GuiState;
use crate::assets::{self, get_asc_svg, get_icon_search, get_logo};
use crate::collection::processor::TradelistProcessor;
use crate::collection::view::table::CollectionTable;
use crate::collection::TableMessage;
use crate::menubar::McmMenuBar;
use crate::util::first_file_match_from_dir;
use crate::{
    appdata, collection, Message, DEFAULT_APP_HEIGHT, DEFAULT_APP_WIDTH, MENU_BAR_HEIGHT,
    MIN_APP_HEIGHT, MIN_APP_WIDTH,
};
use fltk::enums::{CallbackTrigger, Event, Font, FrameType, Shortcut};
use fltk::frame::Frame;
use fltk::image::{Image, PngImage, TiledImage};
use fltk::misc::Progress;
use fltk::prelude::WidgetExt;
use fltk::text::TextAttr;
use fltk::window::DoubleWindow;
use fltk::{app, button, enums::Color, prelude::*, window::Window};
use fltk::{prelude::*, *};
use fltk_flex::{Flex, FlexType};
use fltk_grid::Grid;
use fltk_table::{SmartTable, TableOpts};
use fltk_theme::{widget_themes, ThemeType, WidgetTheme};

use self::setup::setup_main_window;

mod setup;

/// [MtgoGui] is the main GUI struct that holds all the widgets and state for the application
pub struct MtgoGui {
    app: app::App,
    state: GuiState,
    rcv: app::Receiver<Message>,
    main_win: window::Window,
    menu: McmMenuBar,
    collection: CollectionTable,
    tradelist_processor: TradelistProcessor,
}

impl Default for MtgoGui {
    fn default() -> Self {
        Self::new()
    }
}

impl MtgoGui {
    /// Create a new [MtgoGui] instance
    pub fn new() -> Self {
        let app = app::App::default();
        let theme = WidgetTheme::new(ThemeType::Dark);
        theme.apply();

        let (ev_send, ev_rcv) = app::channel();
        let mut main_win: DoubleWindow = setup::setup_main_window();

        let menu = McmMenuBar::new(DEFAULT_APP_WIDTH, MENU_BAR_HEIGHT, &ev_send);

        let flx_left_col = setup::setup_left_column_flx_box();
        setup::set_left_col_box(ev_send.clone());
        flx_left_col.end();

        let collection = collection::view::set_collection_main_box(ev_send.clone());

        main_win.end();
        main_win.show();

        let tradelist_processor = TradelistProcessor::new(ev_send.clone());

        main_win.set_callback(move |_| {
            if app::event() == Event::Close {
                ev_send.send(Message::Quit);
            }
        });
        Self {
            app,
            state: GuiState::default(), // Placeholder, is overwritten at startup
            rcv: ev_rcv,
            main_win,
            menu,
            collection,
            tradelist_processor,
        }
    }

    /// Perform any startup tasks.
    ///
    /// Runs after all the GUI elements are created. And just before the main event loop starts.
    fn run_startup(&mut self) {
        self.state =
            GuiState::load(appdata::util::appdata_path().expect("Failed to get appdata path"))
                .expect("Failed to load GUI state");
        match appdata::util::current_tradelist_path() {
            Ok(Some(current_trade_list)) => {
                self.tradelist_processor.process(current_trade_list.into())
            }
            Err(e) => {
                // TODO - Error pop-up dialog if fails.
                log::error!("Error getting current trade list path: {e}");
            }
            Ok(None) => {
                log::info!("No current trade list found");
            }
        }
    }

    /// Run the application.
    ///
    /// Runs the startup tasks and the main event loop.
    ///
    /// This function will block until the application is closed.
    pub fn run(&mut self) {
        self.run_startup();
        self.gui_main_event_loop();
    }

    /// The main event loop for the application
    fn gui_main_event_loop(&mut self) {
        while self.app.wait() {
            if let Some(msg) = self.rcv.recv() {
                match msg {
                    Message::Quit => {
                        log::info!("Quit");
                        if let Err(e) = self.state.save(
                            appdata::util::appdata_path().expect("Failed to get appdata path"),
                        ) {
                            log::error!("Failed to save GUI state: {e}");
                        }
                        self.app.quit();
                    }
                    Message::MenuBar(mb_msg) => self.menu.handle_ev(mb_msg),
                    Message::Example => {
                        log::info!("Example");
                        let cards: Vec<mtgoupdater::mtgo_card::MtgoCard> =
                            mtgoupdater::internal_only::get_example_card_collection();
                        self.collection.set_cards(cards);
                    }
                    Message::Table(t_m) => {
                        self.collection.handle_ev(t_m);
                        self.app.redraw();
                    }
                    Message::GotFullTradeList(full_trade_list_path) => {
                        // TODO: Error pop-up dialog if fails.
                        // Should implement a generic error dialog that can be used for all unexpected errors that cannot be handled programmatically.
                        appdata::util::copy_tradelist_to_appdata(full_trade_list_path.as_os_str())
                            .unwrap();
                        self.state.new_tradelist();
                        self.tradelist_processor.process(full_trade_list_path);
                    }
                    Message::SetCards(cards) => self.collection.set_cards(cards),
                }
            }
        }
    }
}
