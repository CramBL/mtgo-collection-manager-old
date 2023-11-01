use std::vec::Drain;

use super::util::{MultiValueStat, UniqueTotal};

pub struct BrowserItems {
    item_index: usize,
    formatted_items: Vec<String>,
    title_format: String,
    alt_title_format: String,
    value_format: String,
    alt_value_format: String,
}

impl BrowserItems {
    const ALTERNATING_BACKGROUND_COLOR: &'static str = r#"@B49"#; // Dark grey

    const TITLE_FONT_SIZE: &'static str = r#"@S15"#;
    const TITLE_FONT_BOLD: &'static str = r#"@b"#;

    const VALUE_FONT_SIZE: &'static str = r#"@S13"#;
    const VALUE_FONT_TYPE: &'static str = r#"@F13"#;

    fn title_format(&self) -> &str {
        if self.item_index % 2 == 0 {
            &self.title_format
        } else {
            &self.alt_title_format
        }
    }

    fn value_format(&self) -> &str {
        if self.item_index % 2 == 0 {
            &self.value_format
        } else {
            &self.alt_value_format
        }
    }

    pub fn new() -> Self {
        Self {
            item_index: 0,
            formatted_items: Vec::new(),
            title_format: format!(
                "{font_size}{bold}@.",
                bold = Self::TITLE_FONT_BOLD,
                font_size = Self::TITLE_FONT_SIZE
            ),
            alt_title_format: format!(
                "{font_size}{bold}{background_color}@.",
                bold = Self::TITLE_FONT_BOLD,
                font_size = Self::TITLE_FONT_SIZE,
                background_color = Self::ALTERNATING_BACKGROUND_COLOR
            ),
            value_format: format!(
                "{font_size}{font_type}",
                font_size = Self::VALUE_FONT_SIZE,
                font_type = Self::VALUE_FONT_TYPE
            ),
            alt_value_format: format!(
                "{font_size}{font_type}{background_color}",
                font_size = Self::VALUE_FONT_SIZE,
                font_type = Self::VALUE_FONT_TYPE,
                background_color = Self::ALTERNATING_BACKGROUND_COLOR
            ),
        }
    }

    pub fn add_item(&mut self, title: &str, value: &str) {
        let formatted_item = format!(
            "{title_format}{title}\t{value_format}@.{value}",
            title_format = self.title_format(),
            value_format = self.value_format(),
            title = title,
            value = value
        );
        self.formatted_items.push(formatted_item);
        self.item_index += 1;
    }

    pub fn add_item_unique_total(&mut self, title: &str, unique_total_pair: UniqueTotal) {
        let formatted_item = format!(
            "{title_format}{title}\t{value_format}@.{unique} ({total})",
            title_format = self.title_format(),
            value_format = self.value_format(),
            title = title,
            unique = unique_total_pair.unique(),
            total = unique_total_pair.total()
        );
        self.formatted_items.push(formatted_item);
        self.item_index += 1;
    }

    pub fn add_multi_value_item(&mut self, mut stat: MultiValueStat) {
        let mut first_item = format!(
            "{title_format}{title}\t{value_format}",
            title_format = self.title_format(),
            value_format = self.value_format(),
            title = stat.title()
        );

        let values = stat.take_values();
        let mut value_iter = values.iter();
        if let Some(first_value) = value_iter.next() {
            first_item.push_str(first_value);
        }
        self.formatted_items.push(first_item);
        // Now add any remaining values
        for value in value_iter {
            let formatted_item = format!(
                "{title_format} \t{value_format}{value}",
                title_format = self.title_format(),
                value_format = self.value_format(),
                value = value
            );
            self.formatted_items.push(formatted_item);
        }
        self.item_index += 1;
    }

    pub fn drain(&mut self) -> Drain<'_, String> {
        self.formatted_items.drain(..)
    }
}
