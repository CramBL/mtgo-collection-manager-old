use std::path::PathBuf;

use crate::{util::center, Message};
use fltk::{
    app, dialog,
    enums::{self, Color, Font, FrameType, Shortcut},
    menu,
    prelude::*,
    text::{self, TextAttr},
    window::Window,
};
use fltk_flex::Flex;
use mtgoupdater::{
    mtgo_preprocessor_api::run_mtgo_preprocessor_version, mtgogetter_api::mtgogetter_version,
};

/// Messages that can be received by the menubar
#[derive(Debug, Clone, Copy)]
pub enum MbMessage {
    Open,
    Quit,
    About,
    Example,
}

// Styling for the about window text
const TEXT_ABOUT_STYLES: &[text::StyleTableEntryExt] = &[
    text::StyleTableEntryExt {
        color: Color::White,
        font: Font::HelveticaBold,
        size: 20,
        // defaults
        attr: TextAttr::None,
        bgcolor: Color::Background2,
    },
    text::StyleTableEntryExt {
        color: Color::from_hex(0xA8A8A8),
        font: Font::Helvetica,
        size: 18,
        attr: TextAttr::Underline,
        bgcolor: Color::Background2, // default
    },
    text::StyleTableEntryExt {
        color: Color::Yellow,
        font: Font::Courier,
        size: 16,
        // defaults
        attr: TextAttr::None,
        bgcolor: Color::Background2,
    },
    text::StyleTableEntryExt {
        color: Color::DarkBlue,
        font: Font::HelveticaItalic,
        size: 16,
        // defaults
        attr: TextAttr::None,
        bgcolor: Color::Background2,
    },
];

pub(super) struct McmMenuBar {
    pub(super) menu: menu::SysMenuBar,
    ev_emitter: app::Sender<Message>,
}

impl McmMenuBar {
    pub fn new(w: i32, h: i32, s: &app::Sender<Message>) -> Self {
        let mut mb = menu::SysMenuBar::default().with_size(w, h);
        init_menu_bar(&mut mb, s);
        Self {
            menu: mb,
            ev_emitter: s.clone(),
        }
    }

    pub fn handle_ev(&mut self, ev: MbMessage) {
        match ev {
            MbMessage::Open => self.open_full_tradelist(),
            MbMessage::Quit => app::quit(),
            MbMessage::About => show_about(),
            MbMessage::Example => todo!("example"),
        }
    }

    fn open_full_tradelist(&mut self) {
        let mut dlg = dialog::FileDialog::new(dialog::FileDialogType::BrowseFile);
        dlg.set_option(dialog::FileDialogOptions::NoOptions);
        dlg.set_filter("MTGO Full Trade List\t*.{txt,dek}");
        dlg.show();
        let filename = dlg.filename();
        if !filename.to_string_lossy().to_string().is_empty() {
            if filename.is_file() {
                eprintln!("Full trade list: {:?}", filename);
                self.ev_emitter.send(Message::GotFullTradeList(
                    filename.to_str().expect("Path is invalid unicode").into(),
                ));
            } else {
                dialog::alert(center().0 - 200, center().1 - 100, "File does not exist!")
            }
        }
    }
}

/// Get version information from all components and display in a pop-up window
pub(super) fn show_about() {
    // Start by getting the versions from MTGO Getter and MTGO Preprocessor
    let mtgogetter_version = match mtgogetter_version() {
        Ok(v) => v,
        Err(e) => {
            eprintln!("Error getting mtgogetter version: {e}");
            dialog::alert(
                center().0 - 200,
                center().1 - 100,
                &format!("Could not find MTGO Getter binary!\n{e}"),
            );
            return;
        }
    };
    let mtgo_preproc_version = match run_mtgo_preprocessor_version() {
        Ok(v) => v,
        Err(e) => {
            eprintln!("Error getting mtgo preprocessor version: {e}");
            dialog::alert(
                center().0 - 200,
                center().1 - 100,
                &format!("Could not find MTGO Preprocessor binary!\n{e}"),
            );
            return;
        }
    };

    let mtgogetter_version_str = String::from_utf8_lossy(&mtgogetter_version.stdout);
    let version_pos = mtgogetter_version_str.trim().find("version ").unwrap();
    let just_v = &mtgogetter_version_str[version_pos + 8..];
    let preproc_version_str = String::from_utf8_lossy(&mtgo_preproc_version.stdout)
        .trim()
        .to_string();

    let mtgoupdater_version = mtgoupdater::mtgo_updater_version();

    let mtgo_gui_version = env!("CARGO_PKG_VERSION");

    let project_url = "https://github.com/CramBL/mtgo-collection-manager/";

    let w_width = 400;
    let mut tbuf = text::TextBuffer::default();
    let mut sbuf = text::TextBuffer::default();
    let mtgo_cm_txt = format!(
        "{:^width$}\n",
        "MTGO Collection Manager",
        width = 65 - "MTGO Collection Manager".len()
    );
    let mtgo_cm_ver_txt = format!(
        "{:^width$}\n\n",
        format!("v{}", mtgo_gui_version),
        width = 57 - format!("v{}", mtgo_gui_version).len()
    );
    tbuf.set_text(&mtgo_cm_txt);
    sbuf.set_text(&"A".repeat(mtgo_cm_txt.len()));
    tbuf.append(&mtgo_cm_ver_txt);
    sbuf.append(&"A".repeat(mtgo_cm_ver_txt.len()));
    tbuf.append("Components:\n");
    sbuf.append(&"B".repeat("Components:\n".len()));
    let component_left_pad = 20;
    let mtgogetter_txt = format!(
        "   {:<width$} v{}",
        "MTGO Getter",
        just_v,
        width = component_left_pad
    );
    tbuf.append(&mtgogetter_txt);
    sbuf.append(&"C".repeat(mtgogetter_txt.len()));
    let mtgopreproc_txt = format!(
        "   {:<width$} {}\n",
        "MTGO Preprocessor",
        preproc_version_str,
        width = component_left_pad
    );
    tbuf.append(&mtgopreproc_txt);
    sbuf.append(&"C".repeat(mtgopreproc_txt.len()));
    let mtgoupdater_txt = format!(
        "   {:<width$} v{}\n\n",
        "MTGO Updater",
        mtgoupdater_version,
        width = component_left_pad
    );
    tbuf.append(&mtgoupdater_txt);
    sbuf.append(&"C".repeat(mtgoupdater_txt.len()));
    tbuf.append("Homepage:\n");
    sbuf.append(&"B".repeat("Homepage:\n".len()));
    tbuf.append(project_url);
    sbuf.append(&"D".repeat(project_url.len()));

    let txt_lines = tbuf.count_lines(0, tbuf.length());
    eprintln!("txt_lines: {}", txt_lines);

    let mut win = Window::default()
        .with_size(w_width, txt_lines * 30)
        .with_pos(center().0 - 300, center().1 - 100)
        .with_label(&format!(
            "About MTGO Collection Manager v{}",
            mtgo_gui_version
        ));
    win.set_icon(Some(crate::assets::get_logo()));
    let flex_about = Flex::default()
        .with_pos(0, 0)
        .with_align(enums::Align::Center)
        .size_of_parent()
        .column();

    let mut txt_disp = text::TextDisplay::default();
    txt_disp.align();
    txt_disp.set_buffer(tbuf);
    txt_disp.set_highlight_data_ext(sbuf, TEXT_ABOUT_STYLES);
    txt_disp.set_align(enums::Align::Center);
    txt_disp.wrap_mode(text::WrapMode::AtBounds, 0);
    txt_disp.set_text_color(Color::White);

    flex_about.end();
    win.end();
    win.show();
}

fn init_menu_bar(menu: &mut menu::SysMenuBar, s: &fltk::app::Sender<Message>) {
    menu.set_frame(FrameType::FlatBox);

    menu.add_emit(
        "&File/Open Full Trade list...\t",
        Shortcut::Ctrl | 'o',
        menu::MenuFlag::Normal,
        s.clone(),
        MbMessage::Open.into(),
    );

    menu.add_emit(
        "&File/Quit\t",
        Shortcut::Ctrl | 'q',
        menu::MenuFlag::Normal,
        s.clone(),
        Message::Quit,
    );

    menu.add_emit(
        "&Help/About\t",
        Shortcut::None,
        menu::MenuFlag::Normal,
        s.clone(),
        MbMessage::About.into(),
    );

    if let Some(mut item) = menu.find_item("&File/Quit\t") {
        item.set_label_color(Color::Red);
    }
}
