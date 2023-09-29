use std::sync::OnceLock;

use fltk::image::PngImage;

// Logo placed left-most in window labels
const MCM_LOGO_RAW: &[u8; 3542] = include_bytes!("../assets/35x35-logo-card-pile.png");
pub static MCM_LOGO: OnceLock<PngImage> = OnceLock::new();
pub fn get_logo() -> PngImage {
    MCM_LOGO
        .get_or_init(|| PngImage::from_data(MCM_LOGO_RAW).unwrap())
        .clone()
}

pub fn center() -> (i32, i32) {
    (
        (fltk::app::screen_size().0 / 2.0) as i32,
        (fltk::app::screen_size().1 / 2.0) as i32,
    )
}
