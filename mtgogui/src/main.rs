#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")]
#![allow(unused_imports)]
#![allow(dead_code)]

use std::ffi::OsStr;
use std::path::PathBuf;
use std::sync::OnceLock;

use assets::{get_asc_svg, get_icon_search, get_logo};
use flexi_logger::{Cleanup, Criterion, Duplicate, Naming};
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

mod appdata;
mod assets;
mod collection;
mod gui;
mod menubar;
mod util;

use collection::view::table;
use collection::view::table::column;
use collection::TableMessage;
use gui::MtgoGui;
use menubar::McmMenuBar;
use mtgoupdater::mtgo_card::MtgoCard;
use mtgoupdater::mtgo_preprocessor_api::run_mtgo_preprocessor_version;
use mtgoupdater::mtgogetter_api::mtgogetter_version;

use crate::util::center;

pub const MIN_APP_WIDTH: i32 = 400;
pub const MIN_APP_HEIGHT: i32 = 400;
pub const DEFAULT_APP_WIDTH: i32 = 1400;
pub const DEFAULT_APP_HEIGHT: i32 = 800;
pub const WIDGET_PADDING: i32 = 0;

pub const MENU_BAR_HEIGHT: i32 = 25;

/// Messages for the main event loop
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
    // In debug mode use the paths to the binaries when they're built in each subproject
    if cfg!(debug_assertions) {
        mtgoupdater::internal_only::dev_try_init_mtgogetter_bin();
        mtgoupdater::internal_only::dev_try_init_mtgoparser_bin();
        // Show box edges
        Flex::debug(true);
    }

    use flexi_logger::{FileSpec, Logger, WriteMode};

    let mut appdata_dir = std::env::current_exe().unwrap();
    log::info!("Path to executable: {appdata_dir:?}");
    appdata_dir.pop();
    appdata_dir.push("appdata");
    appdata_dir.push("log_files");
    log::info!("Path to log files: {appdata_dir:?}");

    const FIVE_MI_B: u64 = 5 * 1024 * 1024;
    let _logger = Logger::try_with_str("info")
        .expect("Failed setting up logger")
        .log_to_file(
            FileSpec::default()
                .directory(appdata_dir)
                .basename("mcm_log"),
        )
        .rotate(
            // If the program runs long enough,
            Criterion::Size(FIVE_MI_B), // - create a new file every day
            Naming::Timestamps,         // - let the rotated files have a timestamp in their name
            Cleanup::KeepLogFiles(7),   // - keep at most 7 log files
        )
        .duplicate_to_stderr(Duplicate::Info)
        .write_mode(WriteMode::Async)
        .start()
        .expect("Failed to initialize logger");
    log::info!("Setup GUI");
    let mut gui = MtgoGui::default();
    log::info!("Starting GUI");
    gui.run();
}
