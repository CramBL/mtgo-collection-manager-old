use fltk::{
    browser::Browser,
    prelude::{BrowserExt, WidgetBase, WidgetExt},
};

use super::{container::CollectionStats, items::BrowserItems};

pub struct StatsView {
    browser: Browser,
}

impl StatsView {
    pub fn new(w: i32, h: i32) -> Self {
        let mut browser = Browser::new(0, 0, w, h, "");
        browser.set_column_widths(&[160, 300]);
        browser.set_column_char('\t');

        browser
            .set_tooltip("Values in the pattern of 'X_NUM (Y_NUM)' shows the number of different cards in 'X_NUM' and the total quantity in 'Y_NUM'.\ni.e if you have 4x of one card, it will show '1 (4)'");

        Self { browser }
    }

    pub fn set_stats(&mut self, mut stats: CollectionStats) {
        self.browser.clear();
        let mut browser_items = BrowserItems::new();
        browser_items.add_item("dek-File added", stats.file_from());
        browser_items.add_item_unique_total("Total items", stats.total_cards());
        browser_items
            .add_multi_value_item(stats.take_total_value().expect("No total value stat set"));
        browser_items.add_multi_value_item(
            stats
                .take_most_expensive_item()
                .expect("No most expensive item stat set"),
        );
        browser_items.add_item_unique_total("Cards > 5 tix", stats.cards_over_5_tix());
        browser_items.add_item_unique_total("Cards < 0.1 tix", stats.cards_under_a_tenth_tix());
        browser_items.add_multi_value_item(
            stats
                .take_rarity_distribution()
                .expect("No rarity distribution stat set"),
        );

        for item in browser_items.drain() {
            self.browser.add(&item);
        }
    }
}

impl Default for StatsView {
    fn default() -> Self {
        Self::new(400, 400)
    }
}
