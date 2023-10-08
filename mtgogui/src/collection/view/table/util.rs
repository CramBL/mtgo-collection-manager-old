use mtgoupdater::mtgo_card::MtgoCard;

use crate::collection::{Category, CurrentSortedBy, Direction};

pub struct ColumnStyle {
    pub name: &'static str,
    pub width: i32,
    pub idx: i32,
}

impl ColumnStyle {
    pub const fn new(idx: i32, name: &'static str, width: i32) -> Self {
        Self { idx, name, width }
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
