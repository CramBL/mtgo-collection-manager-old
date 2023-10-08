use std::sync::OnceLock;

use fltk::image::{PngImage, SvgImage};

const ASC_SVG: &str = include_str!("../assets/sortASC.svg");
const DESC_SVG: &str = include_str!("../assets/sortDESC.svg");
static ASC_IMG: OnceLock<SvgImage> = OnceLock::new();
static DESC_IMG: OnceLock<SvgImage> = OnceLock::new();
pub fn get_asc_svg() -> &'static SvgImage {
    ASC_IMG.get_or_init(|| {
        SvgImage::from_data(ASC_SVG).expect("Failed to decode ascending sort order SVG")
    })
}
pub fn get_desc_svg() -> &'static SvgImage {
    DESC_IMG.get_or_init(|| {
        SvgImage::from_data(DESC_SVG).expect("Failed to decode descending sort order SVG")
    })
}

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

pub fn first_file_match_from_dir(
    f_name: &str,
    path: &std::path::Path,
    max_file_age_secs: Option<u64>,
) -> Option<std::path::PathBuf> {
    let mut matching_entries: Vec<std::path::PathBuf> = Vec::new();

    for entry in path.read_dir().unwrap() {
        let dir_entry = entry.unwrap();

        let metadata = std::fs::metadata(&dir_entry.path()).unwrap();
        let last_modified = metadata.modified().unwrap().elapsed().unwrap().as_secs();
        if metadata.is_file() {
            if let Some(max_file_age) = max_file_age_secs {
                if last_modified > max_file_age {
                    continue;
                }
            }
            eprintln!(
                "Name: {}, Path:{}",
                dir_entry.path().file_name().unwrap().to_str().unwrap(),
                dir_entry.path().display()
            );

            if dir_entry
                .file_name()
                .to_owned()
                .to_str()
                .unwrap()
                .contains(f_name)
            {
                matching_entries.push(dir_entry.path());
            }
        }
    }
    if !matching_entries.is_empty() {
        Some(matching_entries[0].clone())
    } else {
        None
    }
}
