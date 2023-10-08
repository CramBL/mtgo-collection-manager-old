use std::sync::OnceLock;

use fltk::{
    image::{PngImage, SvgImage},
    prelude::ImageExt,
};

// Logo placed left-most in window labels
const MCM_LOGO_SVG: &str = include_str!("../assets/logo-small.svg");
// Ascending / Descending symbols
const ASC_SVG: &str = include_str!("../assets/sortASC.svg");
const DESC_SVG: &str = include_str!("../assets/sortDESC.svg");

static MCM_LOGO: OnceLock<SvgImage> = OnceLock::new();
static ASC_IMG: OnceLock<SvgImage> = OnceLock::new();
static DESC_IMG: OnceLock<SvgImage> = OnceLock::new();

/// Returns the sort ascending symbol as a borrowed [SvgImage]
pub fn get_asc_svg() -> &'static SvgImage {
    ASC_IMG.get_or_init(|| {
        SvgImage::from_data(ASC_SVG).expect("Failed to decode ascending sort order SVG")
    })
}
/// Returns the sort descending symbol as a borrowed [SvgImage]
pub fn get_desc_svg() -> &'static SvgImage {
    DESC_IMG.get_or_init(|| {
        SvgImage::from_data(DESC_SVG).expect("Failed to decode descending sort order SVG")
    })
}

/// Returns the MTGO Collection manager symbol as an owned [SvgImage] with scaling set.
pub fn get_logo() -> SvgImage {
    let mut logo = MCM_LOGO
        .get_or_init(|| SvgImage::from_data(MCM_LOGO_SVG).expect("Failed to decode MCM logo SVG"))
        .clone();
    logo.scale(15, 15, true, true);
    logo
}
