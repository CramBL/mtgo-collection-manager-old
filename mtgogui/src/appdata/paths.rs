use std::{
    ffi::OsStr,
    io::Error,
    path::{Path, PathBuf},
};

use crate::util::first_file_match_from_dir;

use super::APP_DATA_DIR;

#[derive(Debug)]
pub struct AppdataPaths {
    appdata_dir: &'static OsStr,
    // Same as above but includes the trailing '/' that is needed in some cases
    appdata_dir_path: String,
    scryfall: PathBuf,
    card_definitions: PathBuf,
    price_history: PathBuf,
}

impl AppdataPaths {
    /// Instiatiate [AppdataPaths] from the path to the appdata directory
    ///
    /// Fails if not all the expected files can be located
    pub fn new() -> Result<Self, Error> {
        let appdata_dir = Path::new(APP_DATA_DIR);
        if !appdata_dir.exists() {
            eprintln!("App data path doesn't exist:{APP_DATA_DIR}");
            return Err(Error::new(
                std::io::ErrorKind::InvalidInput,
                format!("App data path {APP_DATA_DIR} doesn't exist!"),
            ));
        }
        let scryfall_path =
            if let Some(p) = first_file_match_from_dir("scryfall", appdata_dir, None) {
                p
            } else {
                eprintln!("Could not locate Scryfall data json in {APP_DATA_DIR}");
                return Err(Error::new(
                    std::io::ErrorKind::NotFound,
                    format!("Could not find Scryfall data JSON in {APP_DATA_DIR}"),
                ));
            };

        let card_definitions_path =
            if let Some(p) = first_file_match_from_dir("card-def", appdata_dir, None) {
                p
            } else {
                eprintln!("Could not locate card definition json in {APP_DATA_DIR}");
                return Err(Error::new(
                    std::io::ErrorKind::NotFound,
                    format!("Could not find card definition JSON in {APP_DATA_DIR}"),
                ));
            };
        let price_history_path =
            if let Some(p) = first_file_match_from_dir("price-his", appdata_dir, None) {
                p
            } else {
                eprintln!("Could not locate price history json");
                return Err(Error::new(
                    std::io::ErrorKind::NotFound,
                    format!("Could not find price history JSON in {APP_DATA_DIR}"),
                ));
            };

        Ok(Self {
            appdata_dir: OsStr::new(APP_DATA_DIR),
            appdata_dir_path: format!("{APP_DATA_DIR}/"),
            scryfall: scryfall_path,
            card_definitions: card_definitions_path,
            price_history: price_history_path,
        })
    }

    /// Get the path to the appdata dir with a trailing '/'
    pub fn appdata_dir_path(&self) -> &OsStr {
        OsStr::new(&self.appdata_dir_path)
    }

    pub fn scryfall_path(&self) -> &OsStr {
        self.scryfall.as_os_str()
    }

    pub fn card_definitions_path(&self) -> &OsStr {
        self.card_definitions.as_os_str()
    }

    pub fn price_history_path(&self) -> &OsStr {
        self.price_history.as_os_str()
    }
}
