use std::sync::{Arc, Mutex};

use fltk::{
    app, button,
    enums::{self, Event},
    prelude::{GroupExt, WidgetBase, WidgetExt},
    widget_extends,
};
use fltk_flex::Flex;

use crate::{
    collection::{
        view::table::{CollectionTable, SortToggle},
        Category, CtMessage, CurrentSortedBy,
    },
    Message,
};

pub mod table;

const TABLE_WIDTH: i32 = 790;

pub fn set_collection_main_box(ev_send: app::Sender<Message>) -> table::CollectionTable {
    let mut flx_table = Flex::default()
        .with_pos(400, 35)
        .with_size(1000, 600)
        .column();
    flx_table.set_align(enums::Align::LeftTop);

    let mut flx_header = Flex::default()
        .with_pos(0, 0)
        .with_size(TABLE_WIDTH, 0)
        .row();
    flx_header.set_align(enums::Align::RightTop);

    const BTN_SORT_PADDING: i32 = 1;
    flx_header.set_pad(BTN_SORT_PADDING);

    use Category::*;
    let ord: Arc<Mutex<CurrentSortedBy>> = Arc::new(Mutex::new(CurrentSortedBy::None));

    let mut btn_sort_name = SortToggle::new("Name", ord.clone());
    btn_sort_name.emit(ev_send.clone(), CtMessage::SortBy(Name).into());
    let mut btn_sort_quant = SortToggle::new("#", ord.clone());
    btn_sort_quant.emit(ev_send.clone(), CtMessage::SortBy(Quantity).into());
    let mut btn_sort_foil = SortToggle::new("Foil", ord.clone());
    btn_sort_foil.emit(ev_send.clone(), CtMessage::SortBy(Foil).into());
    let mut btn_sort_goatbots = SortToggle::new("Goatbots", ord.clone());
    btn_sort_goatbots.emit(ev_send.clone(), CtMessage::SortBy(Goatbots).into());
    let mut btn_sort_cardhoarder = SortToggle::new("Cardhoarder", ord.clone());
    btn_sort_cardhoarder.emit(ev_send.clone(), CtMessage::SortBy(Scryfall).into());
    let mut btn_sort_set = SortToggle::new("Set", ord.clone());
    btn_sort_set.emit(ev_send.clone(), CtMessage::SortBy(Set).into());
    let mut btn_sort_rarity = SortToggle::new("Rarity", ord.clone());
    btn_sort_rarity.emit(ev_send.clone(), CtMessage::SortBy(Rarity).into());

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
    let collection_table = table::CollectionTable::new(TABLE_WIDTH, 720, ev_send, ord);
    flx_table.end();

    collection_table
}

fn btn_with_emit<T: Into<Option<&'static str>>>(
    ev_send: app::Sender<Message>,
    label: T,
    msg: Message,
) -> button::Button {
    let mut btn = button::Button::new(0, 0, 0, 0, label);
    btn.emit(ev_send, msg);
    btn
}
