pub mod view;

use fltk::{
    app::{self, App},
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
use view::table;

use crate::Message;

#[derive(Debug, Clone)]
pub enum TableMessage {
    SortBy(table::column::Column),
    Search(Box<str>),
}
