#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")]
#![allow(unused_imports)]
#![allow(dead_code)]

use std::ffi::OsStr;
use std::path::PathBuf;
use std::sync::OnceLock;

use assets::{get_asc_svg, get_icon_search, get_logo};
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

mod assets;
mod collection;
mod menubar;
mod mtgogui;
mod util;

use collection::view::table;
use collection::view::table::column;
use collection::TableMessage;
use menubar::McmMenuBar;
use mtgogui::MtgoGui;
use mtgoupdater::mtgo_card::MtgoCard;
use mtgoupdater::mtgo_preprocessor_api::run_mtgo_preprocessor_version;
use mtgoupdater::mtgogetter_api::mtgogetter_version;

use crate::util::center;

// Directory that stores all collection data
pub const APP_DATA_DIR: &str = "appdata";

pub const MIN_APP_WIDTH: i32 = 400;
pub const MIN_APP_HEIGHT: i32 = 400;
pub const DEFAULT_APP_WIDTH: i32 = 1400;
pub const DEFAULT_APP_HEIGHT: i32 = 800;
pub const WIDGET_PADDING: i32 = 0;

#[derive(Debug, Clone)]
pub enum Message {
    Quit,
    Example,
    MenuBar(menubar::MenubarMessage),
    Table(collection::TableMessage),
    GotFullTradeList(Box<std::path::Path>),
    SetCards(Vec<MtgoCard>),
}

impl From<menubar::MenubarMessage> for Message {
    fn from(mb_msg: menubar::MenubarMessage) -> Self {
        Message::MenuBar(mb_msg)
    }
}

impl From<collection::TableMessage> for Message {
    fn from(ct_msg: collection::TableMessage) -> Self {
        Message::Table(ct_msg)
    }
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
