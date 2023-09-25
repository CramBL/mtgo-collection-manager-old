#![allow(unused_imports)]
#![allow(dead_code)]

use fltk::enums::Event;
use fltk::{app, button, enums::Color, prelude::*, window::Window};
use fltk::{prelude::*, *};
use fltk_flex::{Flex, FlexType};
use fltk_grid::Grid;
use fltk_theme::{widget_themes, ThemeType, WidgetTheme};

use mtgoupdater::mtgo_preprocessor_api::run_mtgo_preprocessor_version;
use mtgoupdater::mtgogetter_api::mtgogetter_version;

#[derive(Default)]
struct MyButton {
    btn: button::Button,
}

impl MyButton {
    pub fn new(x: i32, y: i32, w: i32, h: i32, label: &str) -> Self {
        let mut btn = button::Button::new(x, y, w, h, None).with_label(label);
        btn.set_frame(widget_themes::OS_DEFAULT_BUTTON_UP_BOX);
        btn.handle(|b, ev| match ev {
            Event::Enter => {
                b.set_frame(widget_themes::OS_BUTTON_UP_BOX);
                b.redraw();
                true
            }
            Event::Leave => {
                b.set_frame(widget_themes::OS_DEFAULT_BUTTON_UP_BOX);
                b.redraw();
                true
            }
            _ => false,
        });
        Self { btn }
    }
}

fltk::widget_extends!(MyButton, button::Button, btn);

fn main() {
    mtgoupdater::internal_only::dev_try_init_mtgogetter_bin();
    mtgoupdater::internal_only::dev_try_init_mtgoparser_bin();

    let a = app::App::default();

    let theme = WidgetTheme::new(ThemeType::Dark);
    theme.apply();

    let mut win = Window::default()
        .with_size(1000, 600)
        .with_label("MTGO Collection Manager");

    win.set_color(Color::White);

    let f_width = 400;
    let f_height = 500;
    let mut flex = Flex::default().with_size(f_width, f_height).column();

    let mut btn_getter =
        button::Button::new(0, 0, 100, 100, None).with_label("MTGO Getter version");
    let mut btn_preproc =
        button::Button::new(0, 0, 100, 100, None).with_label("MTGO Preprocessor version");

    btn_getter.set_frame(widget_themes::OS_DEFAULT_BUTTON_UP_BOX);

    flex.end();
    Flex::debug(true);

    win.end();
    win.show();

    btn_getter.set_callback({
        let mut win = win.clone();
        move |b| {
            let mtgogetter_version = mtgogetter_version().unwrap();
            let version_str = String::from_utf8_lossy(&mtgogetter_version.stdout);
            eprintln!("{version_str}");
            b.set_label(&version_str);
            win.set_label("Got Getter");
        }
    });

    btn_preproc.set_callback(move |b| {
        let mtgo_preproc_version = run_mtgo_preprocessor_version().unwrap();
        let version_str = String::from_utf8_lossy(&mtgo_preproc_version.stdout)
            .trim()
            .to_string();
        let preprocess_version_str = format!("Preprocessor {}", version_str);
        eprintln!("{preprocess_version_str}");
        b.set_label(&preprocess_version_str);
        win.set_label("Got Preprocessor");
    });

    a.run().unwrap();
}
