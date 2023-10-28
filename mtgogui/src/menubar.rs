use crate::{assets, util::center, Message, DEFAULT_APP_WIDTH, MENU_BAR_HEIGHT, MIN_APP_WIDTH};
use fltk::{
    app::{self, Sender},
    dialog::{self, FileDialog, FileDialogOptions, FileDialogType},
    enums::{self, Align, Color, Font, FrameType, Shortcut},
    menu::{self, MenuFlag},
    misc::Progress,
    prelude::*,
    text::{TextAttr, TextBuffer, TextDisplay, WrapMode},
    window::Window,
};
use fltk_flex::Flex;
use mtgoupdater::{
    mtgo_preprocessor_api::run_mtgo_preprocessor_version, mtgogetter_api::mtgogetter_version,
};
use std::{io, path::PathBuf};
use util::ProgressUpdate;

mod about;
mod setup;
pub mod util;

/// Messages that can be received by the menubar
#[derive(Debug, Clone)]
pub enum MenubarMessage {
    Open,
    Quit,
    About,
    Example,
    ProgressBar(ProgressUpdate),
}

/// The menubar for the application
pub struct McmMenuBar {
    menu: menu::SysMenuBar,
    ev_emitter: app::Sender<Message>,
    progress_bar: Progress,
}

impl McmMenuBar {
    /// Creates a new menubar
    ///
    /// # Arguments
    ///
    /// * `w` - Width of the menubar
    /// * `h` - Height of the menubar
    /// * `s` - Sender to send messages to the main thread
    pub fn new(w: i32, h: i32, s: &app::Sender<Message>) -> Self {
        let mut mb = menu::SysMenuBar::default().with_size(w, h);
        setup::init_menu_bar(&mut mb, s);

        let progress_bar_width = 300;
        let mut progress = Progress::new(
            DEFAULT_APP_WIDTH - progress_bar_width,
            0,
            progress_bar_width,
            MENU_BAR_HEIGHT,
            "Progress bar title",
        );
        progress.set_color(Color::White);
        progress.set_selection_color(Color::Green);
        progress.set_frame(FrameType::FlatBox);
        progress.set_color(Color::Background2);
        progress.set_maximum(100.);
        progress.set_value(0.);
        progress.hide();

        Self {
            menu: mb,
            ev_emitter: s.clone(),
            progress_bar: progress,
        }
    }

    /// Handles events sent to the menubar
    ///
    /// # Arguments
    ///
    /// * `ev` - The event to handle
    pub fn handle_ev(&mut self, ev: MenubarMessage) {
        match ev {
            MenubarMessage::Open => self.open_full_tradelist(),
            MenubarMessage::Quit => app::quit(),
            MenubarMessage::About => about::show_about(),
            MenubarMessage::Example => todo!("example"),
            MenubarMessage::ProgressBar(update) => {
                if update.show {
                    self.progress_bar.show();
                    self.progress_bar.redraw();
                    self.progress_bar.set_value(update.progress);
                    self.progress_bar.set_label(&update.label);
                } else {
                    self.progress_bar.hide();
                }
            }
        }
    }

    fn open_full_tradelist(&mut self) {
        let mut dlg = FileDialog::new(FileDialogType::BrowseFile);
        dlg.set_option(FileDialogOptions::NoOptions);
        dlg.set_filter("MTGO Full Trade List\t*.{txt,dek}");
        dlg.show();
        let filename = dlg.filename();
        if !filename.to_string_lossy().to_string().is_empty() {
            if filename.is_file() {
                log::info!("Full trade list: {:?}", filename);
                self.ev_emitter
                    .send(Message::GotFullTradeList(filename.into()));
            } else {
                dialog::alert(center().0 - 200, center().1 - 100, "File does not exist!")
            }
        }
    }
}
