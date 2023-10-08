use crate::{
    collection::{Category, CurrentSortedBy, Direction},
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
            .with_align(Align::Left | Align::Inside);
        //b.set_down_frame(FrameType::FlatBox);
        b.set_selection_color(Color::color_average(b.color(), Color::Foreground, 0.9));
        b.clear_visible_focus();
        b.set_label_size(app::font_size() - 2);
        b.draw(move |b| {
            if b.value() {
                let mut image = if ord.lock().unwrap().is_descending() {
                    crate::util::get_desc_svg().clone()
                } else {
                    crate::util::get_asc_svg().clone()
                };
                image.scale(15, 15, true, true);
                image.draw(b.x() + (b.w() * 2 / 3) + 5, b.y() + 10, b.w() / 3, b.h());
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
    sorted_by: Arc<Mutex<CurrentSortedBy>>,
}

impl CollectionTable {
    const COL_NAME: ColumnStyle = ColumnStyle::new(0, "NAME", 300);
    const COL_QUANTITY: ColumnStyle = ColumnStyle::new(1, "Quantity", 45);
    const COL_FOIL: ColumnStyle = ColumnStyle::new(2, "FOIL", 45);
    const COL_GOATBOTS: ColumnStyle = ColumnStyle::new(3, "GOATBOTS", 100);
    const COL_CARDHOARDER: ColumnStyle = ColumnStyle::new(4, "CARDHOARDER", 100);
    const COL_SET: ColumnStyle = ColumnStyle::new(5, "SET", 45);
    const COL_RARITY: ColumnStyle = ColumnStyle::new(6, "RARITY", 95);

    pub fn new(
        w: i32,
        h: i32,
        ev_sender: app::Sender<Message>,
        sorted_by: Arc<Mutex<CurrentSortedBy>>,
    ) -> Self {
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
            sorted_by,
        }
    }

    pub fn handle_ev(&mut self, ev: CtMessage) {
        match ev {
            CtMessage::SortBy(cat) => {
                println!("sort by {:?}", cat);
                let new_order =
                    util::sort_cards(&mut self.cards, cat, *self.sorted_by.lock().unwrap());
                *self.sorted_by.lock().unwrap() = new_order;
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
