use std::{ffi::OsString, io, path::PathBuf};

use chrono::prelude::*;
use serde_derive::{Deserialize, Serialize};
use toml::Table;

use super::GUI_STATE;

#[derive(Debug, Serialize, Deserialize, Clone, Default, PartialEq, Eq)]
pub struct GuiState {
    tradelist_added_date: DateTime<Utc>,
}

impl GuiState {
    /// Create a new [GuiState] instance
    pub fn new() -> Self {
        Self::default()
    }

    /// Save the [GuiState] to the given directory as a TOML file.
    ///
    /// # Arguments
    ///
    /// * `dst_dir` - The directory to save the [GuiState] to.
    ///
    /// # Errors
    ///
    /// Returns an [io::Error] if the [GuiState] fails to be saved.
    pub fn save(&self, mut dst_dir: PathBuf) -> io::Result<()> {
        dst_dir.push(GUI_STATE);
        let toml = toml::to_string(&self).expect("Failed to serialize GUI state");
        std::fs::write(dst_dir, toml)?;
        Ok(())
    }

    /// Load the [GuiState] from the given directory containing a TOML file named [GUI_STATE] or create a default [GuiState] if there's no matching file in the directory.
    ///
    /// # Arguments
    ///
    /// * `src_dir` - The directory to load the [GuiState] from.
    ///
    /// # Errors
    ///
    /// Returns an [io::Error] if the [GuiState] fails to be loaded.
    pub fn load(mut src_dir: PathBuf) -> io::Result<Self> {
        src_dir.push(GUI_STATE);
        let gui_state = if src_dir.exists() {
            let toml = std::fs::read_to_string(src_dir)?;
            toml::from_str(&toml).expect("Failed to deserialize GUI state")
        } else {
            log::info!("No GUI state found, creating default GUI state");
            Self::new()
        };
        Ok(gui_state)
    }

    /// Save the current [DateTime<Utc>] as the last time a tradelist was added.
    pub fn new_tradelist(&mut self) {
        self.tradelist_added_date = Utc::now();
    }

    /// Get the [DateTime<Utc>] of the last time a tradelist was added.
    pub fn get_tradelist_added_date(&self) -> DateTime<Utc> {
        self.tradelist_added_date
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use pretty_assertions::assert_eq;
    use temp_dir::TempDir;

    #[test]
    fn test_gui_state_load_save() {
        let tmpdir = TempDir::new().unwrap();
        let tmpdir_path = tmpdir.path().to_path_buf();

        let gui_state = GuiState::new();

        gui_state.save(tmpdir_path.clone()).unwrap();
        let gui_state_loaded = GuiState::load(tmpdir_path).unwrap();

        assert_eq!(gui_state, gui_state_loaded);
    }

    #[test]
    fn test_gui_state_tradelist_data() {
        let mut gui_state = GuiState::new();
        let now = Utc::now();
        gui_state.tradelist_added_date = now;

        let after = now + chrono::Duration::seconds(1);

        assert!(gui_state.tradelist_added_date < after);
    }

    #[test]
    fn test_gui_state_tradelist_data_serde() {
        let tmpdir = TempDir::new().unwrap();
        let tmpdir_path = tmpdir.path().to_path_buf();

        let mut gui_state = GuiState::new();
        let now = Utc::now();
        gui_state.tradelist_added_date = now;

        gui_state.save(tmpdir_path.clone()).unwrap();
        let gui_state_loaded = GuiState::load(tmpdir_path).unwrap();

        assert_eq!(gui_state, gui_state_loaded);
        assert_eq!(gui_state_loaded.tradelist_added_date, now);
        assert!(gui_state_loaded.tradelist_added_date < now + chrono::Duration::seconds(1));
    }
}
