use std::ffi::OsStr;
use std::path::PathBuf;
use std::sync::OnceLock;

use crate::assets::{self, get_asc_svg, get_icon_search, get_logo};
use crate::collection::processor::TradelistProcessor;
use crate::collection::view::table::CollectionTable;
use crate::collection::TableMessage;
use crate::menubar::McmMenuBar;
use crate::util::first_file_match_from_dir;
use crate::{
    collection, Message, DEFAULT_APP_HEIGHT, DEFAULT_APP_WIDTH, MIN_APP_HEIGHT, MIN_APP_WIDTH,
};
use fltk::enums::{CallbackTrigger, Event, Font, FrameType, Shortcut};
use fltk::image::{Image, PngImage, TiledImage};
use fltk::prelude::WidgetExt;
use fltk::text::TextAttr;
use fltk::window::DoubleWindow;
use fltk::{app, button, enums::Color, prelude::*, window::Window};
use fltk::{prelude::*, *};
use fltk_flex::{Flex, FlexType};
use fltk_grid::Grid;
use fltk_table::{SmartTable, TableOpts};
use fltk_theme::{widget_themes, ThemeType, WidgetTheme};

mod setup;

pub struct MtgoGui {
    app: app::App,
    full_tradelist: Option<PathBuf>,
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
    pub fn new() -> Self {
        let app = app::App::default();
        let theme = WidgetTheme::new(ThemeType::Dark);
        theme.apply();

        let (ev_send, ev_rcv) = app::channel();
        let mut main_win: DoubleWindow = Window::default()
            .with_size(DEFAULT_APP_WIDTH, DEFAULT_APP_HEIGHT)
            .center_screen()
            .with_label("MTGO Collection Manager");

        main_win.set_icon(Some(assets::get_logo()));
        main_win.make_resizable(true);
        main_win.size_range(MIN_APP_WIDTH, MIN_APP_HEIGHT, 0, 0);
        main_win.set_color(Color::Black);
        let menu = McmMenuBar::new(DEFAULT_APP_WIDTH, 25, &ev_send);

        setup::set_left_col_box(ev_send.clone());
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
            full_tradelist: None,
            rcv: ev_rcv,
            main_win,
            menu,
            collection,
            tradelist_processor,
        }
    }

    pub fn run(&mut self) {
        while self.app.wait() {
            if let Some(msg) = self.rcv.recv() {
                match msg {
                    Message::Quit => {
                        log::info!("Quit");
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
                        self.tradelist_processor.process(full_trade_list_path);
                    }
                    Message::SetCards(cards) => self.collection.set_cards(cards),
                }
            }
        }
    }
}
