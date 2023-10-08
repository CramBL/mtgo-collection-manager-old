use fltk::{app, group::Column};
use fltk_table::{SmartTable, TableOpts};
use mtgoupdater::mtgo_card::MtgoCard;
use std::fmt::Alignment;

use crate::{
    collection::{Category, CurrentSortedBy, Direction},
    Message,
};
use fltk::{
    app::App,
    enums::{Align, Color, Event, FrameType},
    image,
    prelude::*,
    prelude::{GroupExt, TableExt, WidgetExt},
};

use super::CtMessage;

mod util;
use util::ColumnStyle;

pub struct CollectionTable {
    pub(super) table: SmartTable,
    pub(super) cards: Vec<MtgoCard>,
    sorted_by: CurrentSortedBy,
}

impl CollectionTable {
    const COL_NAME: ColumnStyle = ColumnStyle::new(0, "NAME", 300);
    const COL_QUANTITY: ColumnStyle = ColumnStyle::new(1, "Quantity", 45);
    const COL_FOIL: ColumnStyle = ColumnStyle::new(2, "FOIL", 45);
    const COL_GOATBOTS: ColumnStyle = ColumnStyle::new(3, "GOATBOTS", 100);
    const COL_CARDHOARDER: ColumnStyle = ColumnStyle::new(4, "CARDHOARDER", 100);
    const COL_SET: ColumnStyle = ColumnStyle::new(5, "SET", 45);
    const COL_RARITY: ColumnStyle = ColumnStyle::new(6, "RARITY", 95);

    pub fn new(w: i32, h: i32, ev_sender: app::Sender<Message>) -> Self {
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
        table.handle({
            let mut dnd = false;
            let mut released = false;
            move |_, ev| match ev {
                Event::DndEnter => {
                    dnd = true;
                    true
                }
                Event::DndDrag => true,
                Event::DndRelease => {
                    released = true;
                    true
                }
                Event::Paste => {
                    if dnd && released {
                        let path = app::event_text();
                        eprintln!("path: {}", path);
                        ev_sender.send(Message::GotFullTradeList(path.into()));
                        dnd = false;
                        released = false;
                        true
                    } else {
                        false
                    }
                }
                Event::DndLeave => {
                    dnd = false;
                    released = false;
                    true
                }
                _ => false,
            }
        });

        Self {
            table,
            cards: vec![],
            sorted_by: CurrentSortedBy::None,
        }
    }

    pub fn handle_ev(&mut self, ev: CtMessage) {
        match ev {
            CtMessage::SortBy(cat) => {
                println!("sort by {:?}", cat);
                self.sorted_by = util::sort_cards(&mut self.cards, cat, self.sorted_by);
                self.draw_cards();
            }
        }
    }

    pub fn set_cards(&mut self, cards: Vec<MtgoCard>) {
        self.cards = cards;
        self.draw_cards();
    }

    fn draw_cards(&mut self) {
        if self.cards.is_empty() {
            return;
        }
        // Don't run this gui on some platform with usize < u32 if you're gonna make a huge table
        if self.cards.len() > self.table.row_count() as usize {
            for _ in 0..(self.cards.len() - self.table.row_count() as usize) {
                self.table.append_empty_row("");
            }
        }
        // Fill all the rows with cards data
        for (i, c) in self.cards.iter().enumerate() {
            let row_idx = i as i32;
            self.table.set_cell_value(row_idx, 0, &c.name);
            self.table
                .set_cell_value(row_idx, 1, &c.quantity.to_string());
            self.table
                .set_cell_value(row_idx, 2, if c.foil { "Yes" } else { "No" });
            self.table
                .set_cell_value(row_idx, 3, &format!("{:8.3}", c.goatbots_price));
            self.table.set_cell_value(row_idx, 4, &{
                if let Some(p) = c.scryfall_price {
                    p.to_string()
                } else {
                    "N/A".into()
                }
            });
            self.table.set_cell_value(row_idx, 5, &c.set);
            self.table.set_cell_value(row_idx, 6, &c.rarity.to_string());
        }
    }
}
