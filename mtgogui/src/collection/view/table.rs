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
    const COL_RARITY: ColumnStyle = ColumnStyle::new(6, "RARITY", 45);

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

        // table.set_col_header_value(0, "NAME");
        // table.set_col_header_value(1, "QUANTITY");
        // table.set_col_header_value(2, "FOIL");
        // table.set_col_header_value(3, "GOATBOTS");
        // table.set_col_header_value(4, "SCRYFALL");
        // table.set_col_header_value(5, "SET");
        // table.set_col_header_value(6, "RARITY");

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
                match cat {
                    Category::Name => {
                        if self.sorted_by == CurrentSortedBy::Name(Direction::Ascending) {
                            self.cards.sort_by(|a, b| b.name.cmp(&a.name));
                            self.sorted_by = CurrentSortedBy::Name(Direction::Descending);
                        } else {
                            self.cards.sort_by(|a, b| a.name.cmp(&b.name));
                            self.sorted_by = CurrentSortedBy::Name(Direction::Ascending);
                        }
                    }
                    Category::Quantity => {
                        if self.sorted_by == CurrentSortedBy::Quantity(Direction::Ascending) {
                            self.cards.sort_by(|a, b| b.quantity.cmp(&a.quantity));
                            self.sorted_by = CurrentSortedBy::Quantity(Direction::Descending);
                        } else {
                            self.cards.sort_by(|a, b| a.quantity.cmp(&b.quantity));
                            self.sorted_by = CurrentSortedBy::Quantity(Direction::Ascending);
                        }
                    }
                    Category::Foil => {
                        if self.sorted_by == CurrentSortedBy::Foil(Direction::Ascending) {
                            self.cards.sort_by(|a, b| b.foil.cmp(&a.foil));
                            self.sorted_by = CurrentSortedBy::Foil(Direction::Descending);
                        } else {
                            self.cards.sort_by(|a, b| a.foil.cmp(&b.foil));
                            self.sorted_by = CurrentSortedBy::Foil(Direction::Ascending);
                        }
                    }
                    Category::Goatbots => {
                        if self.sorted_by == CurrentSortedBy::Goatbots(Direction::Ascending) {
                            self.cards.sort_by(|a, b| {
                                b.goatbots_price.partial_cmp(&a.goatbots_price).unwrap()
                            });
                            self.sorted_by = CurrentSortedBy::Goatbots(Direction::Descending);
                        } else {
                            self.cards.sort_by(|a, b| {
                                a.goatbots_price.partial_cmp(&b.goatbots_price).unwrap()
                            });
                            self.sorted_by = CurrentSortedBy::Goatbots(Direction::Ascending);
                        }
                    }
                    Category::Scryfall => {
                        if self.sorted_by == CurrentSortedBy::Scryfall(Direction::Ascending) {
                            self.cards.sort_by(|a, b| {
                                b.scryfall_price.partial_cmp(&a.scryfall_price).unwrap()
                            });
                            self.sorted_by = CurrentSortedBy::Scryfall(Direction::Descending);
                        } else {
                            self.cards.sort_by(|a, b| {
                                a.scryfall_price.partial_cmp(&b.scryfall_price).unwrap()
                            });
                            self.sorted_by = CurrentSortedBy::Scryfall(Direction::Ascending);
                        }
                    }
                    Category::Set => {
                        if self.sorted_by == CurrentSortedBy::Set(Direction::Ascending) {
                            self.cards.sort_by(|a, b| b.set.cmp(&a.set));
                            self.sorted_by = CurrentSortedBy::Set(Direction::Descending);
                        } else {
                            self.cards.sort_by(|a, b| a.set.cmp(&b.set));
                            self.sorted_by = CurrentSortedBy::Set(Direction::Ascending);
                        }
                    }
                    Category::Rarity => {
                        if self.sorted_by == CurrentSortedBy::Rarity(Direction::Ascending) {
                            self.cards.sort_by(|a, b| b.rarity.cmp(&a.rarity));
                            self.sorted_by = CurrentSortedBy::Rarity(Direction::Descending);
                        } else {
                            self.cards.sort_by(|a, b| a.rarity.cmp(&b.rarity));
                            self.sorted_by = CurrentSortedBy::Rarity(Direction::Ascending);
                        }
                    }
                }
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
