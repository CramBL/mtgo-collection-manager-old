use fltk::{
    app, button,
    enums::{self, Align, CallbackTrigger, Color},
    frame, input,
    prelude::{DisplayExt, GroupExt, ImageExt, InputExt, WidgetBase, WidgetExt, WindowExt},
    text::{TextBuffer, TextDisplay, WrapMode},
    window::{DoubleWindow, Window},
};
use fltk_flex::Flex;
use fltk_grid::Grid;

use crate::{
    assets::{self, get_icon_search},
    collection::TableMessage,
    Message, DEFAULT_APP_HEIGHT, DEFAULT_APP_WIDTH, MIN_APP_HEIGHT, MIN_APP_WIDTH,
};

/// Sets up the left column of the application
///
/// # Arguments
///
/// * `ev_send` - Sender to send messages to the main thread
pub(super) fn set_left_col_box(ev_send: app::Sender<Message>) -> TextDisplay {
    let mut search_box_grid_row = Grid::new(0, 0, 400, 30, "");
    if cfg!(debug_assertions) {
        // Show box edges and coordinates
        search_box_grid_row.show_grid(true);
    }
    search_box_grid_row.set_layout(10, 4);
    let mut frame = frame::Frame::new(0, 0, 0, 10, "");
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

    search_box_grid_row.set_widget(&mut frame, 0, 0);
    search_box_grid_row.set_widget(&mut search_input, 0, 1..4);

    let mut frame_tradelist_age = frame::Frame::new(0, 0, 0, 10, "Tradelist set:");
    let mut txt_disp_tradelist_age = TextDisplay::default();
    txt_disp_tradelist_age.set_align(Align::Center | Align::Inside);
    txt_disp_tradelist_age.wrap_mode(WrapMode::AtBounds, 0);

    search_box_grid_row.set_widget(&mut frame_tradelist_age, 1, 0);
    search_box_grid_row.set_widget(&mut txt_disp_tradelist_age, 1, 1..4);

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
    txt_disp_tradelist_age
}

/// Sets up the main window of the application
///
/// # Returns
///
/// The main window of the application as a [DoubleWindow]
pub(super) fn setup_main_window() -> DoubleWindow {
    let mut main_win: DoubleWindow = Window::default()
        .with_size(DEFAULT_APP_WIDTH, DEFAULT_APP_HEIGHT)
        .center_screen()
        .with_label("MTGO Collection Manager");

    main_win.set_icon(Some(assets::get_logo()));
    main_win.make_resizable(true);
    main_win.size_range(MIN_APP_WIDTH, MIN_APP_HEIGHT, 0, 0);
    main_win.set_color(Color::Black);
    main_win
}

/// Sets up the left column [Flex] box of the application
///
/// # Returns
///
/// The left column [Flex] box of the application
pub(super) fn setup_left_column_flx_box() -> Flex {
    let mut flx_left_col = Flex::default().with_pos(0, 35).with_size(400, 600).column();
    flx_left_col.set_align(enums::Align::LeftTop);
    flx_left_col
}
