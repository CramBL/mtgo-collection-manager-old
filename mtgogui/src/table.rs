use fltk::enums::Color;
use fltk_table::{SmartTable, TableOpts};

pub(super) struct CollectionTable {
    pub(super) table: SmartTable,
}

impl CollectionTable {
    pub fn new(w: i32, h: i32) -> Self {
        let mut table = SmartTable::default()
            .with_size(w, h)
            .center_of_parent()
            .with_opts(TableOpts {
                rows: 0,
                cols: 7,
                editable: false,
                cell_font_color: Color::White,

                ..Default::default()
            });

        table.set_col_header_value(0, "NAME");
        table.set_col_width(0, 300);
        table.set_col_header_value(1, "QUANTITY");
        table.set_col_header_value(2, "FOIL");
        table.set_col_width(2, 45);
        table.set_col_header_value(3, "GOATBOTS");
        table.set_col_width(3, 100);
        table.set_col_header_value(4, "SCRYFALL");
        table.set_col_width(4, 100);
        table.set_col_header_value(5, "SET");
        table.set_col_width(5, 45);
        table.set_col_header_value(6, "RARITY");
        Self { table }
    }
}

impl Default for CollectionTable {
    fn default() -> Self {
        Self::new(790, 590)
    }
}
