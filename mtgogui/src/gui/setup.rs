use fltk::{
    app, button,
    enums::{self, CallbackTrigger},
    frame, input,
    prelude::{GroupExt, ImageExt, InputExt, WidgetBase, WidgetExt},
};
use fltk_flex::Flex;
use fltk_grid::Grid;

use crate::{assets::get_icon_search, collection::TableMessage, Message};

pub(super) fn set_left_col_box(ev_send: app::Sender<Message>) {
    let mut flx_left_col = Flex::default().with_pos(0, 35).with_size(400, 600).column();
    flx_left_col.set_align(enums::Align::LeftTop);

    let mut search_box_grid_row = Grid::new(0, 0, 400, 30, "");
    if cfg!(debug_assertions) {
        // Show box edges and coordinates
        search_box_grid_row.debug(true);
    }
    search_box_grid_row.set_layout(1, 4);
    let mut frame = frame::Frame::new(0, 0, 100, 30, "");
    frame.draw(|f| {
        let mut icon = get_icon_search();
        icon.scale(f.w(), f.h(), true, false);
        icon.draw(f.x(), f.y(), f.w(), f.h());
    });
    let mut search_input = input::Input::default().with_label("Search");
    search_input.set_trigger(CallbackTrigger::Changed);
    search_input.set_callback({
        let s = ev_send.clone();
        move |i| {
            s.send(TableMessage::Search(i.value().into()).into());
        }
    });
    search_box_grid_row.insert(&mut frame, 0, 0);
    search_box_grid_row.insert(&mut search_input, 0, 1..4);

    search_box_grid_row.end();

    if cfg!(debug_assertions) {
        let mut btn_example = button::Button::new(0, 0, 100, 25, "Example");
        btn_example.set_callback({
            move |b| {
                ev_send.send(Message::Example);

                b.set_label("Getting example...");
            }
        });
    }

    flx_left_col.end();
}
