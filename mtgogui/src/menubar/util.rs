use fltk::{
    enums::{Color, Font},
    text::{self, TextAttr},
};

/// Styling for the about window text
pub const TEXT_ABOUT_STYLES: &[text::StyleTableEntryExt] = &[
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

/// Progress bar update message
#[derive(Debug, Clone)]
pub struct ProgressUpdate {
    pub show: bool,
    pub progress: f64,
    pub label: Box<str>,
}
