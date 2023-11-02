use fltk::{
    browser::Browser,
    prelude::{BrowserExt, WidgetBase, WidgetExt},
};

use super::{container::CollectionStats, items::BrowserItems};

pub struct StatsView {
    browser: Browser,
}

impl StatsView {
    /// Creates a new [StatsView] with the given width and height
    pub fn new(w: i32, h: i32) -> Self {
        let mut browser = Browser::new(0, 0, w, h, "");
        browser.set_column_widths(&[160, 300]);
        browser.set_column_char('\t');

        browser
            .set_tooltip("Values in the pattern of 'X_NUM (Y_NUM)' shows the number of different cards in 'X_NUM' and the total quantity in 'Y_NUM'.\ni.e if you have 4x of one card, it will show '1 (4)'");

        Self { browser }
    }

    pub fn browser(&mut self) -> &mut Browser {
        &mut self.browser
    }

    pub fn set_items(&mut self, mut items: BrowserItems) {
        self.browser.clear();
        for item in items.drain() {
            self.browser.add(&item);
        }
    }
}

impl Default for StatsView {
    fn default() -> Self {
        Self::new(400, 400)
    }
}
