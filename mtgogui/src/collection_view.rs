use fltk::{
    app, button, enums,
    prelude::{GroupExt, WidgetBase, WidgetExt},
};
use fltk_flex::Flex;

use crate::{
    table::{self, Category, CtMessage},
    Message,
};

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

    use Category::*;
    let btn_sort_quantity = btn_with_emit(ev_send, "Quantity", CtMessage::SortBy(Quantity).into());
    let btn_srt_name = btn_with_emit(ev_send, "Name", CtMessage::SortBy(Name).into());
    let btn_sort_rarity = btn_with_emit(ev_send, "Rarity", CtMessage::SortBy(Rarity).into());

    flx_header.fixed(&btn_sort_quantity, 100);
    flx_header.fixed(&btn_srt_name, 100);
    flx_header.fixed(&btn_sort_rarity, 100);
    flx_header.end();

    flx_table.fixed(&flx_header, 50);
    let collection_table = table::CollectionTable::new(TABLE_WIDTH, 720);
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
