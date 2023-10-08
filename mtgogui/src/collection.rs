pub mod view;

use std::fmt::Alignment;

use fltk::{
    app::{self, App},
    enums::{Align, Color, Event, FrameType},
    image,
    prelude::*,
    prelude::{GroupExt, TableExt, WidgetExt},
};
use fltk_table::{SmartTable, TableOpts};
use mtgoupdater::mtgo_card::MtgoCard;

use crate::Message;

#[derive(Debug, Clone, Copy)]
pub enum CtMessage {
    SortBy(Category),
}

#[derive(Debug, Clone, Copy)]
pub enum Category {
    Name,
    Quantity,
    Foil,
    Goatbots,
    Scryfall,
    Set,
    Rarity,
}

#[derive(Debug, Clone, Copy, PartialEq)]

pub enum CurrentSortedBy {
    None,
    Name(Direction),
    Quantity(Direction),
    Foil(Direction),
    Goatbots(Direction),
    Scryfall(Direction),
    Set(Direction),
    Rarity(Direction),
}

#[derive(Debug, Clone, Copy, PartialEq)]
pub enum Direction {
    Ascending,
    Descending,
}

impl CurrentSortedBy {
    pub fn is_descending(&self) -> bool {
        if let CurrentSortedBy::None = self {
            return false;
        }
        matches!(self, CurrentSortedBy::Name(d)
                     | CurrentSortedBy::Quantity(d)
                     | CurrentSortedBy::Foil(d)
                     | CurrentSortedBy::Goatbots(d)
                     | CurrentSortedBy::Scryfall(d)
                     | CurrentSortedBy::Set(d)
                     | CurrentSortedBy::Rarity(d) if *d == Direction::Descending)
    }
}
