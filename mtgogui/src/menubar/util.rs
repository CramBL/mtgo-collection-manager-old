use std::io;

use fltk::{
    enums::{Color, Font},
    text::{self, TextAttr, TextBuffer},
};
use mtgoupdater::{
    mtgo_preprocessor_api::run_mtgo_preprocessor_version, mtgogetter_api::mtgogetter_version,
};

/// Progress bar update message
#[derive(Debug, Clone)]
pub struct ProgressUpdate {
    pub show: bool,
    pub progress: f64,
    pub label: Box<str>,
}

/// Get the version of the MTGO Getter binary (X.Y.Z) and return it as a string
///
/// # Errors
/// Returns an error if the MTGO Getter binary cannot be found
pub fn mtgogetter_version_str() -> Result<String, io::Error> {
    let mtgogetter_version = match mtgogetter_version() {
        Ok(v) => v,
        Err(e) => return Err(e),
    };
    let mtgogetter_version_str = String::from_utf8_lossy(&mtgogetter_version.stdout);
    let version_pos = mtgogetter_version_str.trim().find("version ").unwrap();
    Ok(mtgogetter_version_str[version_pos + 8..].to_string())
}

/// Get the version of the MTGO Preprocessor binary (X.Y.Z) and return it as a string
///
/// # Errors
/// Returns an error if the MTGO Preprocessor binary cannot be found
pub fn mtgo_preprocessor_version() -> Result<String, io::Error> {
    let mtgo_preproc_version = match run_mtgo_preprocessor_version() {
        Ok(v) => v,
        Err(e) => return Err(e),
    };
    Ok(String::from_utf8_lossy(&mtgo_preproc_version.stdout)
        .trim()
        .to_string())
}

/// A text buffer and its associated style buffer
#[derive(Debug)]
pub struct TextBufferStylePair {
    text: Option<TextBuffer>,
    style: Option<TextBuffer>,
}

impl TextBufferStylePair {
    /// Create a new [TextBufferStylePair] with the given text and style buffers
    pub fn new(text: TextBuffer, style: TextBuffer) -> Self {
        Self {
            text: Some(text),
            style: Some(style),
        }
    }

    /// Take the text buffer out of the [TextBufferStylePair]
    pub fn text(&mut self) -> TextBuffer {
        self.text.take().expect("Text buffer already taken")
    }

    /// Take the style buffer out of the [TextBufferStylePair]
    pub fn style(&mut self) -> TextBuffer {
        self.style.take().expect("Style buffer already taken")
    }

    /// Get the number of lines in the text buffer
    ///
    /// # Panics
    ///
    /// Panics if any of the buffers have already been taken
    pub fn line_count(&self) -> i32 {
        self.text
            .as_ref()
            .expect("Text buffer already taken")
            .count_lines(
                0,
                self.text
                    .as_ref()
                    .expect("Style buffer already taken")
                    .length(),
            )
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use pretty_assertions::assert_eq;

    #[test]
    fn get_mtgogetter_version() {
        mtgoupdater::internal_only::dev_try_init_mtgogetter_bin();
        mtgoupdater::internal_only::dev_try_init_mtgoparser_bin();

        let mtgogetter_version = mtgogetter_version_str().unwrap();
        assert_eq!(mtgogetter_version, "0.1.0\n");
    }

    #[test]
    fn get_mtgo_preprocessor_version() {
        mtgoupdater::internal_only::dev_try_init_mtgogetter_bin();
        mtgoupdater::internal_only::dev_try_init_mtgoparser_bin();

        let mtgo_preproc_version = mtgo_preprocessor_version().unwrap();
        assert_eq!(mtgo_preproc_version, "v0.1.0");
    }
}
