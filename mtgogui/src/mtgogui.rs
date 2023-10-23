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
    collection, Message, APP_DATA_DIR, DEFAULT_APP_HEIGHT, DEFAULT_APP_WIDTH, MIN_APP_HEIGHT,
    MIN_APP_WIDTH,
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

pub struct MtgoGui {
    app: app::App,
    full_tradelist: Option<PathBuf>,
    rcv: app::Receiver<Message>,
    main_win: window::Window,
    menu: McmMenuBar,
    collection: CollectionTable,
    tradelist_processor: TradelistProcessor,
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

        set_left_col_box(ev_send.clone());
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
                        self.tradelist_processor.process(full_trade_list_path);
                    }
                    Message::SetCards(cards) => self.collection.set_cards(cards),
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

    let mut search_box_grid_row = Grid::new(0, 0, 400, 100, "");
    if cfg!(debug_assertions) {
        search_box_grid_row.debug(true);
    }
    search_box_grid_row.set_layout(1, 4);
    let mut frame = frame::Frame::new(0, 0, 100, 100, "");
    frame.draw(|f| {
        let mut icon = get_icon_search();
        icon.draw(f.x(), f.y(), f.w(), f.h());
    });
    let mut search_input = input::Input::default().with_label("Search");
    search_input.set_trigger(CallbackTrigger::Changed);
    search_input.set_callback({
        let s = ev_send.clone();
        move |i| {
            println!("Got: {}", i.value());
            s.send(TableMessage::Search(i.value().into()).into());
        }
    });
    search_box_grid_row.insert(&mut frame, 0, 0);
    search_box_grid_row.insert(&mut search_input, 0, 1..4);

    search_box_grid_row.end();

    if cfg!(debug_assertions) {
        let mut btn_example = button::Button::new(0, 0, 100, 25, "Example");
        btn_example.set_callback({
            move |b| {
                ev_send.send(Message::Example);

                b.set_label("Getting example...");
            }
        });
    }

    flx_left_col.end();
}
