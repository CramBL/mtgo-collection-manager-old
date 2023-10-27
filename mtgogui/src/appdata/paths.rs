use std::{
    ffi::OsStr,
    io::Error,
    path::{Path, PathBuf},
};

use crate::util::first_file_match_from_dir;

use super::APP_DATA_DIR;

#[derive(Debug)]
pub struct AppData {
    appdata_dir: PathBuf,
    // Same as above but includes the trailing '/' that is needed in some cases
    appdata_dir_path: String,
    scryfall: PathBuf,
    card_definitions: PathBuf,
    price_history: PathBuf,
}

impl AppData {
    /// Instiatiate [AppdataPaths] from the path to the appdata directory
    ///
    /// Fails if not all the expected files can be located
    pub fn new() -> Result<Self, Error> {
        let mut appdata_dir = std::env::current_exe().unwrap();
        log::info!("Path to executable: {appdata_dir:?}");
        appdata_dir.pop();
        if cfg!(windows) {
            appdata_dir.push(format!(r#"{APP_DATA_DIR}\"#));
        } else {
            appdata_dir.push(format!(r#"{APP_DATA_DIR}/"#));
        }
        log::info!("Path to appdata dir: {appdata_dir:?}");

        if !appdata_dir.exists() {
            log::info!("App data path doesn't exist! - {appdata_dir:?}");
            return Err(Error::new(
                std::io::ErrorKind::InvalidInput,
                format!("App data path {APP_DATA_DIR} doesn't exist!"),
            ));
        }

        // Get App Data
        match mtgoupdater::mtgogetter_api::mtgogetter_update_all(appdata_dir.as_os_str()) {
            Ok(output) => {
                log::info!("MTGO Getter output: {}", output.status);
            }
            Err(e) => {
                log::info!("MTGO Getter error: {e}");
            }
        }

        let scryfall_path =
            if let Some(p) = first_file_match_from_dir("scryfall", appdata_dir.as_path(), None) {
                p
            } else {
                log::info!("Could not locate Scryfall data json in {appdata_dir:?}");
                return Err(Error::new(
                    std::io::ErrorKind::NotFound,
                    format!("Could not find Scryfall data JSON in {appdata_dir:?}"),
                ));
            };

        let card_definitions_path =
            if let Some(p) = first_file_match_from_dir("card-def", appdata_dir.as_path(), None) {
                p
            } else {
                log::info!("Could not locate card definition json in {appdata_dir:?}");
                return Err(Error::new(
                    std::io::ErrorKind::NotFound,
                    format!("Could not find card definition JSON in {appdata_dir:?}"),
                ));
            };
        let price_history_path =
            if let Some(p) = first_file_match_from_dir("price-his", appdata_dir.as_path(), None) {
                p
            } else {
                log::info!("Could not locate price history json");
                return Err(Error::new(
                    std::io::ErrorKind::NotFound,
                    format!("Could not find price history JSON in {appdata_dir:?}"),
                ));
            };

        Ok(Self {
            appdata_dir,
            appdata_dir_path: format!("{APP_DATA_DIR}/"),
            scryfall: scryfall_path,
            card_definitions: card_definitions_path,
            price_history: price_history_path,
        })
    }

    /// Get the path to the appdata dir with a trailing '/'
    pub fn appdata_dir_path(&self) -> &OsStr {
        self.appdata_dir.as_os_str()
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
