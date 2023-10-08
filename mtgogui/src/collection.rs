pub mod view;

use std::{
    fmt::Alignment,
    sync::{Arc, Mutex},
};

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
        match self {
            CurrentSortedBy::Name(d)
            | CurrentSortedBy::Quantity(d)
            | CurrentSortedBy::Foil(d)
            | CurrentSortedBy::Goatbots(d)
            | CurrentSortedBy::Scryfall(d)
            | CurrentSortedBy::Set(d)
            | CurrentSortedBy::Rarity(d)
                if *d == Direction::Descending =>
            {
                true
            }
            _ => false,
        }
    }
}

pub struct SortStates {
    pub name: Arc<Mutex<CurrentSortedBy>>,
    pub quantity: Arc<Mutex<CurrentSortedBy>>,
    pub foil: Arc<Mutex<CurrentSortedBy>>,
    pub goatbots: Arc<Mutex<CurrentSortedBy>>,
    pub cardhoarder: Arc<Mutex<CurrentSortedBy>>,
    pub set: Arc<Mutex<CurrentSortedBy>>,
    pub rarity: Arc<Mutex<CurrentSortedBy>>,
}

impl SortStates {
    pub fn new() -> Self {
        // Set all as sorted by ascending as default, as they are not sorted and then the first toggle will sort descending
        Self {
            name: Arc::new(Mutex::new(CurrentSortedBy::Foil(Direction::Ascending))),
            quantity: Arc::new(Mutex::new(CurrentSortedBy::Quantity(Direction::Ascending))),
            foil: Arc::new(Mutex::new(CurrentSortedBy::Foil(Direction::Ascending))),
            goatbots: Arc::new(Mutex::new(CurrentSortedBy::Goatbots(Direction::Ascending))),
            cardhoarder: Arc::new(Mutex::new(CurrentSortedBy::Scryfall(Direction::Ascending))),
            set: Arc::new(Mutex::new(CurrentSortedBy::Set(Direction::Ascending))),
            rarity: Arc::new(Mutex::new(CurrentSortedBy::Rarity(Direction::Ascending))),
        }
    }

    pub fn name_ord(&self) -> CurrentSortedBy {
        *self.name.lock().unwrap()
    }
    pub fn set_name_ord(&mut self, new_ord: CurrentSortedBy) {
        *self.name.lock().unwrap() = new_ord;
    }

    pub fn quantity_ord(&self) -> CurrentSortedBy {
        *self.quantity.lock().unwrap()
    }
    pub fn set_quantity_ord(&mut self, new_ord: CurrentSortedBy) {
        *self.quantity.lock().unwrap() = new_ord;
    }

    pub fn foil_ord(&self) -> CurrentSortedBy {
        *self.foil.lock().unwrap()
    }
    pub fn set_foil_ord(&mut self, new_ord: CurrentSortedBy) {
        *self.foil.lock().unwrap() = new_ord;
    }

    pub fn goatbots_ord(&self) -> CurrentSortedBy {
        *self.goatbots.lock().unwrap()
    }
    pub fn set_goatbots_ord(&mut self, new_ord: CurrentSortedBy) {
        *self.goatbots.lock().unwrap() = new_ord;
    }

    pub fn cardhoarder_ord(&self) -> CurrentSortedBy {
        *self.cardhoarder.lock().unwrap()
    }
    pub fn set_cardhoarder_ord(&mut self, new_ord: CurrentSortedBy) {
        *self.cardhoarder.lock().unwrap() = new_ord;
    }

    pub fn set_ord(&self) -> CurrentSortedBy {
        *self.set.lock().unwrap()
    }
    pub fn set_set_ord(&mut self, new_ord: CurrentSortedBy) {
        *self.set.lock().unwrap() = new_ord;
    }

    pub fn rarity_ord(&self) -> CurrentSortedBy {
        *self.rarity.lock().unwrap()
    }
    pub fn set_rarity_ord(&mut self, new_ord: CurrentSortedBy) {
        *self.rarity.lock().unwrap() = new_ord;
    }
}

impl Default for SortStates {
    fn default() -> Self {
        Self::new()
    }
}
