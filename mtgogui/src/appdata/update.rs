use std::{
    ffi::OsStr,
    io::Error,
    path::{Path, PathBuf},
};

use crate::util::first_file_match_from_dir;

use super::{paths::CardDataPaths, APP_DATA_DIR};

#[derive(Debug)]
pub struct AppData {
    appdata_dir: PathBuf,
    card_data: CardDataPaths,
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

        let card_data_paths = CardDataPaths::find(&appdata_dir)?;

        Ok(Self {
            appdata_dir,
            card_data: card_data_paths,
        })
    }

    /// Get the path to the appdata dir with a trailing '/'
    pub fn appdata_dir_path(&self) -> &OsStr {
        self.appdata_dir.as_os_str()
    }

    pub fn scryfall_path(&self) -> &OsStr {
        self.card_data.scryfall_path().as_os_str()
    }

    pub fn card_definitions_path(&self) -> &OsStr {
        self.card_data.card_definitions_path().as_os_str()
    }

    pub fn price_history_path(&self) -> &OsStr {
        self.card_data.price_history_path().as_os_str()
    }
}
