use fltk::{
    app::{self, Sender},
    enums::Event,
    prelude::{TableExt, WidgetBase},
};
use fltk_table::SmartTable;
use mtgoupdater::mtgo_card::MtgoCard;

use crate::{
    collection::{Category, CurrentSortedBy, Direction},
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

pub fn sort_cards(
    cards: &mut Vec<MtgoCard>,
    category: Category,
    current_sorted: CurrentSortedBy,
) -> CurrentSortedBy {
    match category {
        Category::Name => {
            if current_sorted == CurrentSortedBy::Name(Direction::Ascending) {
                cards.sort_by(|a, b| b.name.cmp(&a.name));
                CurrentSortedBy::Name(Direction::Descending)
            } else {
                cards.sort_by(|a, b| a.name.cmp(&b.name));
                CurrentSortedBy::Name(Direction::Ascending)
            }
        }
        Category::Quantity => {
            if current_sorted == CurrentSortedBy::Quantity(Direction::Ascending) {
                cards.sort_by(|a, b| b.quantity.cmp(&a.quantity));
                CurrentSortedBy::Quantity(Direction::Descending)
            } else {
                cards.sort_by(|a, b| a.quantity.cmp(&b.quantity));
                CurrentSortedBy::Quantity(Direction::Ascending)
            }
        }
        Category::Foil => {
            if current_sorted == CurrentSortedBy::Foil(Direction::Ascending) {
                cards.sort_by(|a, b| b.foil.cmp(&a.foil));
                CurrentSortedBy::Foil(Direction::Descending)
            } else {
                cards.sort_by(|a, b| a.foil.cmp(&b.foil));
                CurrentSortedBy::Foil(Direction::Ascending)
            }
        }
        Category::Goatbots => {
            if current_sorted == CurrentSortedBy::Goatbots(Direction::Ascending) {
                cards.sort_by(|a, b| b.goatbots_price.partial_cmp(&a.goatbots_price).unwrap());
                CurrentSortedBy::Goatbots(Direction::Descending)
            } else {
                cards.sort_by(|a, b| a.goatbots_price.partial_cmp(&b.goatbots_price).unwrap());
                CurrentSortedBy::Goatbots(Direction::Ascending)
            }
        }
        Category::Scryfall => {
            if current_sorted == CurrentSortedBy::Scryfall(Direction::Ascending) {
                cards.sort_by(|a, b| b.scryfall_price.partial_cmp(&a.scryfall_price).unwrap());
                CurrentSortedBy::Scryfall(Direction::Descending)
            } else {
                cards.sort_by(|a, b| a.scryfall_price.partial_cmp(&b.scryfall_price).unwrap());
                CurrentSortedBy::Scryfall(Direction::Ascending)
            }
        }
        Category::Set => {
            if current_sorted == CurrentSortedBy::Set(Direction::Ascending) {
                cards.sort_by(|a, b| b.set.cmp(&a.set));
                CurrentSortedBy::Set(Direction::Descending)
            } else {
                cards.sort_by(|a, b| a.set.cmp(&b.set));
                CurrentSortedBy::Set(Direction::Ascending)
            }
        }
        Category::Rarity => {
            if current_sorted == CurrentSortedBy::Rarity(Direction::Ascending) {
                cards.sort_by(|a, b| b.rarity.cmp(&a.rarity));
                CurrentSortedBy::Rarity(Direction::Descending)
            } else {
                cards.sort_by(|a, b| a.rarity.cmp(&b.rarity));
                CurrentSortedBy::Rarity(Direction::Ascending)
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
