#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")]
#![allow(unused_imports)]
#![allow(dead_code)]

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
use fltk_theme::{widget_themes, ThemeType, WidgetTheme};

mod menubar;
mod util;

use menubar::McmMenuBar;
use mtgoupdater::mtgo_preprocessor_api::run_mtgo_preprocessor_version;
use mtgoupdater::mtgogetter_api::mtgogetter_version;

use crate::util::center;

const MIN_APP_WIDTH: i32 = 400;
const MIN_APP_HEIGHT: i32 = 400;
const DEFAULT_APP_WIDTH: i32 = 1400;
const DEFAULT_APP_HEIGHT: i32 = 800;
const WIDGET_PADDING: i32 = 0;

#[derive(Debug, Clone, Copy)]
enum Message {
    Quit,
    MenuBar(menubar::MbMessage),
}

impl From<menubar::MbMessage> for Message {
    fn from(mb_msg: menubar::MbMessage) -> Self {
        Message::MenuBar(mb_msg)
    }
}

pub struct MtgoGui {
    app: app::App,
    full_tradelist: Option<PathBuf>,
    rcv: app::Receiver<Message>,
    main_win: window::Window,
    menu: McmMenuBar,
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
        let menu = McmMenuBar::new(DEFAULT_APP_WIDTH, 30, &ev_send);
        let mut flx_left_col = Flex::default().with_pos(0, 35).with_size(400, 600).column();
        flx_left_col.set_align(enums::Align::LeftTop);
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

fn main() {
    mtgoupdater::internal_only::dev_try_init_mtgogetter_bin();
    mtgoupdater::internal_only::dev_try_init_mtgoparser_bin();

    let mut gui = MtgoGui::default();
    gui.run();

    // btn_getter.set_callback({
    //     move |b| {
    //         let mtgogetter_version = mtgogetter_version().unwrap();
    //         let version_str = String::from_utf8_lossy(&mtgogetter_version.stdout);
    //         eprintln!("{version_str}");
    //         b.set_label(&version_str);
    //         eprintln!("Got Getter");
    //     }
    // });

    // btn_preproc.set_callback(move |b| {
    //     let mtgo_preproc_version = run_mtgo_preprocessor_version().unwrap();
    //     let version_str = String::from_utf8_lossy(&mtgo_preproc_version.stdout)
    //         .trim()
    //         .to_string();
    //     let preprocess_version_str = format!("Preprocessor {}", version_str);
    //     eprintln!("{preprocess_version_str}");
    //     b.set_label(&preprocess_version_str);
    //     eprintln!("Got Preprocessor");
    // });
}
