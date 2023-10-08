use crate::{
    collection::{Category, CurrentSortedBy, Direction, SortStates},
    Message,
};
use fltk::{app, button, group::Column};
use fltk::{
    app::App,
    enums::{Align, Color, Event, FrameType},
    image,
    prelude::*,
    prelude::{GroupExt, TableExt, WidgetExt},
};
use fltk_table::{SmartTable, TableOpts};
use mtgoupdater::mtgo_card::MtgoCard;
use std::{
    fmt::Alignment,
    sync::{Arc, Mutex},
};

use super::CtMessage;

mod util;
use util::ColumnStyle;

pub struct SortToggle {
    b: button::Button,
}

impl SortToggle {
    pub fn new(label: &str, ord: Arc<Mutex<CurrentSortedBy>>) -> Self {
        let mut b = button::Button::default()
            .with_size(70, 0)
            .with_label(label)
            .with_align(Align::Center | Align::Inside);
        b.set_down_frame(FrameType::FlatBox);
        b.set_selection_color(Color::color_average(b.color(), Color::Foreground, 0.9));
        b.clear_visible_focus();
        b.set_label_size(app::font_size() + 2);
        b.draw(move |b| {
            if b.value() {
                // When a button is clicked, it will show this SVG
                // so it should show what order it's being toggled to
                let mut image = if ord.lock().unwrap().is_descending() {
                    // Toggling to sort by ascending order
                    crate::util::get_asc_svg().clone()
                } else {
                    // Toggling to sort by descending order
                    crate::util::get_desc_svg().clone()
                };
                image.scale(15, 15, true, true);
                image.draw(b.x() + (b.w() - 30) + 5, b.y() + 30, b.w(), b.h());
            }
        });
        b.set_frame(FrameType::FlatBox);
        Self { b }
    }
}

fltk::widget_extends!(SortToggle, button::Button, b);

pub struct CollectionTable {
    pub(super) table: SmartTable,
    pub(super) cards: Vec<MtgoCard>,
    sort_states: SortStates,
}

impl CollectionTable {
    pub const COL_NAME: ColumnStyle = ColumnStyle::new(0, "NAME", 300);
    pub const COL_QUANTITY: ColumnStyle = ColumnStyle::new(1, "Quantity", 60);
    pub const COL_FOIL: ColumnStyle = ColumnStyle::new(2, "FOIL", 60);
    pub const COL_GOATBOTS: ColumnStyle = ColumnStyle::new(3, "GOATBOTS", 100);
    pub const COL_CARDHOARDER: ColumnStyle = ColumnStyle::new(4, "CARDHOARDER", 100);
    pub const COL_SET: ColumnStyle = ColumnStyle::new(5, "SET", 60);
    pub const COL_RARITY: ColumnStyle = ColumnStyle::new(6, "RARITY", 100);

    pub fn new(w: i32, h: i32, ev_sender: app::Sender<Message>, sort_states: SortStates) -> Self {
        // Create the row of buttons to sort by columns

        // Create the table that displays all cards with their info
        let mut table = SmartTable::default()
            .with_size(w, h)
            .center_of_parent()
            .with_opts(TableOpts {
                rows: 0,
                cols: 7,
                editable: false,
                cell_font_color: Color::White,
                header_frame: FrameType::NoBox,
                header_font_color: Color::White,
                ..Default::default()
            });

        table.set_row_header(false);
        table.set_col_header(false);

        table.set_col_width(Self::COL_NAME.idx, Self::COL_NAME.width);
        table.set_col_width(Self::COL_QUANTITY.idx, Self::COL_QUANTITY.width);
        table.set_col_width(Self::COL_FOIL.idx, Self::COL_FOIL.width);
        table.set_col_width(Self::COL_GOATBOTS.idx, Self::COL_GOATBOTS.width);
        table.set_col_width(Self::COL_CARDHOARDER.idx, Self::COL_CARDHOARDER.width);
        table.set_col_width(Self::COL_SET.idx, Self::COL_SET.width);
        table.set_col_width(Self::COL_RARITY.idx, Self::COL_RARITY.width);

        // Support drag-and-drop a full trade list file
        util::set_drag_and_drop_callback(&mut table, ev_sender);

        Self {
            table,
            cards: vec![],
            sort_states,
        }
    }

    pub fn handle_ev(&mut self, ev: CtMessage) {
        match ev {
            CtMessage::SortBy(cat) => {
                println!("sort by {:?}", cat);
                util::sort_cards(&mut self.cards, &mut self.sort_states, cat);
                self.draw_cards();
            }
        }
    }

    pub fn set_cards(&mut self, cards: Vec<MtgoCard>) {
        self.cards = cards;
        self.draw_cards();
    }

    fn draw_cards(&mut self) {
        util::draw_cards(&mut self.table, &self.cards);
    }
}
