use std::sync::{Arc, Mutex};

use crate::{
    collection::{
        view::table::{column::SortStates, CollectionTable, SortToggle},
        TableMessage,
    },
    Message, DEFAULT_APP_HEIGHT, MENU_BAR_HEIGHT,
};
use fltk::{
    app, button,
    enums::{self, Event},
    prelude::{GroupExt, WidgetBase, WidgetExt},
    widget_extends,
};
use fltk_flex::Flex;
use table::column::Column::*;

pub mod table;

const TABLE_WIDTH: i32 = 790;

/// Set the layout for the main collection table to the main window
///
/// Includes setting up all the column headers and their sort buttons
///
/// # Arguments
///
/// * `ev_send` - Sender to send messages to the main thread
///
/// # Returns
///
/// The collection table as a [CollectionTable]
pub fn set_collection_main_box(ev_send: app::Sender<Message>) -> table::CollectionTable {
    let mut flx_table = Flex::default()
        .with_pos(400, 35)
        .with_size(1000, DEFAULT_APP_HEIGHT - MENU_BAR_HEIGHT)
        .column();
    flx_table.set_align(enums::Align::LeftTop);

    let mut flx_header = Flex::default()
        .with_pos(0, 0)
        .with_size(TABLE_WIDTH, 0)
        .row();
    flx_header.set_align(enums::Align::RightTop);

    const BTN_SORT_PADDING: i32 = 1;
    flx_header.set_pad(BTN_SORT_PADDING);

    let sort_states = SortStates::default();

    let mut btn_sort_name = SortToggle::new("Name", sort_states.name.clone());
    btn_sort_name.emit(ev_send.clone(), TableMessage::SortBy(Name).into());
    let mut btn_sort_quant = SortToggle::new("#", sort_states.quantity.clone());
    btn_sort_quant.emit(ev_send.clone(), TableMessage::SortBy(Quantity).into());
    let mut btn_sort_foil = SortToggle::new("Foil", sort_states.foil.clone());
    btn_sort_foil.emit(ev_send.clone(), TableMessage::SortBy(Foil).into());
    let mut btn_sort_goatbots = SortToggle::new("Goatbots", sort_states.goatbots.clone());
    btn_sort_goatbots.emit(ev_send.clone(), TableMessage::SortBy(Goatbots).into());
    let mut btn_sort_cardhoarder = SortToggle::new("Cardhoarder", sort_states.cardhoarder.clone());
    btn_sort_cardhoarder.emit(ev_send.clone(), TableMessage::SortBy(Scryfall).into());
    let mut btn_sort_set = SortToggle::new("Set", sort_states.set.clone());
    btn_sort_set.emit(ev_send.clone(), TableMessage::SortBy(Set).into());
    let mut btn_sort_rarity = SortToggle::new("Rarity", sort_states.rarity.clone());
    btn_sort_rarity.emit(ev_send.clone(), TableMessage::SortBy(Rarity).into());

    flx_header.fixed(
        &*btn_sort_name,
        CollectionTable::COL_NAME.width - BTN_SORT_PADDING,
    );
    flx_header.fixed(
        &*btn_sort_quant,
        CollectionTable::COL_QUANTITY.width - BTN_SORT_PADDING,
    );
    flx_header.fixed(
        &*btn_sort_foil,
        CollectionTable::COL_FOIL.width - BTN_SORT_PADDING,
    );
    flx_header.fixed(
        &*btn_sort_goatbots,
        CollectionTable::COL_GOATBOTS.width - BTN_SORT_PADDING,
    );
    flx_header.fixed(
        &*btn_sort_cardhoarder,
        CollectionTable::COL_CARDHOARDER.width - BTN_SORT_PADDING,
    );
    flx_header.fixed(
        &*btn_sort_set,
        CollectionTable::COL_SET.width - BTN_SORT_PADDING,
    );
    flx_header.fixed(
        &*btn_sort_rarity,
        CollectionTable::COL_RARITY.width - BTN_SORT_PADDING,
    );
    flx_header.end();

    flx_table.fixed(&flx_header, 50);
    let collection_table = table::CollectionTable::new(TABLE_WIDTH, 720, ev_send, sort_states);
    flx_table.end();

    collection_table
}
