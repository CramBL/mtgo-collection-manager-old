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

const ASC_SVG: &str = include_str!("../assets/sortASC.svg");
const DESC_SVG: &str = include_str!("../assets/sortDESC.svg");

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

enum CurrentSortedBy {
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
enum Direction {
    Ascending,
    Descending,
}
