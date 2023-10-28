use std::{
    io::Error,
    path::{Path, PathBuf},
};

use crate::util::first_file_match_from_dir;

#[derive(Debug)]
pub(super) struct CardDataPaths {
    scryfall: PathBuf,
    card_definitions: PathBuf,
    price_history: PathBuf,
}

impl CardDataPaths {
    // Strings used to locate the card data files in the appdata directory
    const FIND_SCRYFALL: &'static str = "scryfall";
    const FIND_CARD_DEFINITIONS: &'static str = "card-def";
    const FIND_PRICE_HISTORY: &'static str = "price-his";

    /// Find the paths to the card data files in the appdata directory
    pub fn find(appdata_dir: &Path) -> Result<Self, Error> {
        let scryfall_path =
            if let Some(p) = first_file_match_from_dir(Self::FIND_SCRYFALL, appdata_dir, None) {
                p
            } else {
                log::info!("Could not locate Scryfall data json in {appdata_dir:?}");
                return Err(Error::new(
                    std::io::ErrorKind::NotFound,
                    format!("Could not find Scryfall data JSON in {appdata_dir:?}"),
                ));
            };

        let card_definitions_path = if let Some(p) =
            first_file_match_from_dir(Self::FIND_CARD_DEFINITIONS, appdata_dir, None)
        {
            p
        } else {
            log::info!("Could not locate card definition json in {appdata_dir:?}");
            return Err(Error::new(
                std::io::ErrorKind::NotFound,
                format!("Could not find card definition JSON in {appdata_dir:?}"),
            ));
        };
        let price_history_path = if let Some(p) =
            first_file_match_from_dir(Self::FIND_PRICE_HISTORY, appdata_dir, None)
        {
            p
        } else {
            log::info!("Could not locate price history json");
            return Err(Error::new(
                std::io::ErrorKind::NotFound,
                format!("Could not find price history JSON in {appdata_dir:?}"),
            ));
        };

        Ok(CardDataPaths {
            scryfall: scryfall_path,
            card_definitions: card_definitions_path,
            price_history: price_history_path,
        })
    }

    pub fn scryfall_path(&self) -> &Path {
        self.scryfall.as_path()
    }

    pub fn card_definitions_path(&self) -> &Path {
        self.card_definitions.as_path()
    }

    pub fn price_history_path(&self) -> &Path {
        self.price_history.as_path()
    }
}
