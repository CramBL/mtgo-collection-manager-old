use std::sync::OnceLock;

use fltk::{
    image::{PngImage, SvgImage},
    prelude::ImageExt,
};

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
const MCM_LOGO_SVG: &str = include_str!("../assets/logo-small.svg");
pub static MCM_LOGO: OnceLock<SvgImage> = OnceLock::new();
pub fn get_logo() -> SvgImage {
    let mut logo = MCM_LOGO
        .get_or_init(|| SvgImage::from_data(MCM_LOGO_SVG).expect("Failed to decode MCM logo SVG"))
        .clone();
    logo.scale(15, 15, true, true);
    logo
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

            if dir_entry.file_name().to_string_lossy().contains(f_name) {
                return Some(dir_entry.path());
            }
        }
    }

    None
}

#[cfg(test)]
mod tests {
    use std::path::PathBuf;

    use super::*;

    #[test]
    fn test_find_first_match_cargolock() {
        let cwd = std::env::current_dir().unwrap();
        let first_match = first_file_match_from_dir("Cargo.lock", &cwd, None);

        assert_eq!(
            PathBuf::from("Cargo.lock"),
            first_match.unwrap().file_name().unwrap()
        );
    }

    #[test]
    fn test_find_first_match_cargotoml() {
        let cwd = std::env::current_dir().unwrap();
        let first_match = first_file_match_from_dir("Cargo.toml", &cwd, None);

        assert_eq!(
            PathBuf::from("Cargo.toml"),
            first_match.unwrap().file_name().unwrap()
        );
    }

    #[test]
    fn test_find_first_match_cargo_dot() {
        // Searching for "Cargo." finds Cargo.lock first because it searches alphabetically

        let cwd = std::env::current_dir().unwrap();
        let first_match = first_file_match_from_dir("Cargo.", &cwd, None);

        assert_eq!(
            PathBuf::from("Cargo.lock"),
            first_match.unwrap().file_name().unwrap()
        );
    }
}
