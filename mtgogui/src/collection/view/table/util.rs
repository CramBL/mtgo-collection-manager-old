use fltk::{
    app::{self, Sender},
    enums::Event,
    prelude::{TableExt, WidgetBase},
};
use fltk_table::SmartTable;
use mtgoupdater::mtgo_card::MtgoCard;

use crate::{
    collection::{Category, CurrentSortedBy, Direction, SortStates},
    Message,
};

use super::CollectionTable;

pub struct ColumnStyle {
    pub name: &'static str,
    pub width: i32,
    pub idx: i32,
}

impl ColumnStyle {
    /// Declare a new column with an associated index, name, and width
    pub const fn new(idx: i32, name: &'static str, width: i32) -> Self {
        Self { idx, name, width }
    }

    /// Assign `val` to a cell in the column at `row_idx`
    pub fn fill(&self, table: &mut SmartTable, row_idx: i32, val: &str) {
        table.set_cell_value(row_idx, self.idx, val);
    }
}

pub fn sort_cards(cards: &mut Vec<MtgoCard>, sort_states: &mut SortStates, category: Category) {
    match category {
        Category::Name => {
            if sort_states.name_ord().is_descending() {
                cards.sort_by(|a, b| b.name.cmp(&a.name));
                sort_states.set_name_ord(CurrentSortedBy::Name(Direction::Ascending));
            } else {
                cards.sort_by(|a, b| a.name.cmp(&b.name));
                sort_states.set_name_ord(CurrentSortedBy::Name(Direction::Descending));
            }
        }
        Category::Quantity => {
            if sort_states.quantity_ord().is_descending() {
                cards.sort_by(|a, b| a.quantity.cmp(&b.quantity));
                sort_states.set_quantity_ord(CurrentSortedBy::Quantity(Direction::Ascending));
            } else {
                cards.sort_by(|a, b| b.quantity.cmp(&a.quantity));
                sort_states.set_quantity_ord(CurrentSortedBy::Quantity(Direction::Descending));
            }
        }
        Category::Foil => {
            if sort_states.foil_ord().is_descending() {
                cards.sort_by(|a, b| a.foil.cmp(&b.foil));
                sort_states.set_foil_ord(CurrentSortedBy::Foil(Direction::Ascending));
            } else {
                cards.sort_by(|a, b| b.foil.cmp(&a.foil));
                sort_states.set_foil_ord(CurrentSortedBy::Foil(Direction::Descending));
            }
        }
        Category::Goatbots => {
            if sort_states.goatbots_ord().is_descending() {
                cards.sort_by(|a, b| a.goatbots_price.partial_cmp(&b.goatbots_price).unwrap());
                sort_states.set_goatbots_ord(CurrentSortedBy::Goatbots(Direction::Ascending));
            } else {
                cards.sort_by(|a, b| b.goatbots_price.partial_cmp(&a.goatbots_price).unwrap());
                sort_states.set_goatbots_ord(CurrentSortedBy::Goatbots(Direction::Descending));
            }
        }
        Category::Scryfall => {
            if sort_states.cardhoarder_ord().is_descending() {
                cards.sort_by(|a, b| a.scryfall_price.partial_cmp(&b.scryfall_price).unwrap());
                sort_states.set_cardhoarder_ord(CurrentSortedBy::Scryfall(Direction::Ascending));
            } else {
                cards.sort_by(|a, b| b.scryfall_price.partial_cmp(&a.scryfall_price).unwrap());
                sort_states.set_cardhoarder_ord(CurrentSortedBy::Scryfall(Direction::Descending));
            }
        }
        Category::Set => {
            if sort_states.set_ord().is_descending() {
                cards.sort_by(|a, b| b.set.cmp(&a.set));
                sort_states.set_set_ord(CurrentSortedBy::Set(Direction::Ascending));
            } else {
                cards.sort_by(|a, b| a.set.cmp(&b.set));
                sort_states.set_set_ord(CurrentSortedBy::Set(Direction::Descending));
            }
        }
        Category::Rarity => {
            if sort_states.rarity_ord().is_descending() {
                cards.sort_by(|a, b| a.rarity.cmp(&b.rarity));
                sort_states.set_rarity_ord(CurrentSortedBy::Rarity(Direction::Ascending));
            } else {
                cards.sort_by(|a, b| b.rarity.cmp(&a.rarity));
                sort_states.set_rarity_ord(CurrentSortedBy::Rarity(Direction::Descending));
            }
        }
    }
}

/// Drag and drop a file onto the table invokes this callback
///
/// Takes the path to the file and forwards it with the event: [Message::GotFullTradeList]
pub fn set_drag_and_drop_callback(table: &mut SmartTable, ev_sender: Sender<Message>) {
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
}

/// Iterates over the [SmartTable] and the [MtgoCard]s filling out all the cells of the table.
pub fn draw_cards(table: &mut SmartTable, cards: &[MtgoCard]) {
    if cards.is_empty() {
        return;
    }

    // Extend the table with rows matching the amount of cards
    if cards.len() > table.row_count() as usize {
        let cards_minus_rows = cards.len() - table.row_count() as usize;
        for _ in 0..(cards_minus_rows) {
            table.append_empty_row("");
        }
    }

    // Iterate over the rows and filling each column of a row with values from cards
    for (idx, card) in cards.iter().enumerate() {
        let row_idx = idx as i32;
        fill_card_row(table, row_idx, card);
    }
}

/// Helper to fill a single row with [MtgoCard] data
fn fill_card_row(table: &mut SmartTable, row_idx: i32, card: &MtgoCard) {
    CollectionTable::COL_NAME.fill(table, row_idx, &card.name);
    CollectionTable::COL_QUANTITY.fill(table, row_idx, &card.quantity.to_string());
    CollectionTable::COL_FOIL.fill(table, row_idx, if card.foil { "Yes" } else { "No" });
    CollectionTable::COL_GOATBOTS.fill(table, row_idx, &format!("{:8.3}", card.goatbots_price));
    CollectionTable::COL_CARDHOARDER.fill(table, row_idx, &{
        if let Some(p) = card.scryfall_price {
            p.to_string()
        } else {
            "N/A".into()
        }
    });
    CollectionTable::COL_SET.fill(table, row_idx, &card.set);
    CollectionTable::COL_RARITY.fill(table, row_idx, &card.rarity.to_string());
}
