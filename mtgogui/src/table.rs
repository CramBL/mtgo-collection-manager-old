use fltk::{
    app::{self, App},
    enums::Color,
};
use fltk_table::{SmartTable, TableOpts};
use mtgoupdater::mtgo_card::MtgoCard;

#[derive(Debug, Clone, Copy)]
pub enum CtMessage {
    SortBy(Category),
}

#[derive(Debug, Clone, Copy)]
pub enum Category {
    Name,
    Quantity,
    Foil,
    Goatbots,
    Scryfall,
    Set,
    Rarity,
}

#[derive(Debug, Clone, Copy, PartialEq)]

enum CurrentSortedBy {
    None,
    Name(Direction),
    Quantity(Direction),
    Foil(Direction),
    Goatbots(Direction),
    Scryfall(Direction),
    Set(Direction),
    Rarity(Direction),
}

#[derive(Debug, Clone, Copy, PartialEq)]
enum Direction {
    Ascending,
    Descending,
}

pub(super) struct CollectionTable {
    pub(super) table: SmartTable,
    pub(super) cards: Vec<MtgoCard>,
    sorted_by: CurrentSortedBy,
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

        Self {
            table,
            cards: vec![],
            sorted_by: CurrentSortedBy::None,
        }
    }

    pub fn handle_ev(&mut self, ev: CtMessage) {
        match ev {
            CtMessage::SortBy(cat) => {
                println!("sort by {:?}", cat);
                match cat {
                    Category::Name => {
                        if self.sorted_by == CurrentSortedBy::Name(Direction::Ascending) {
                            self.cards.sort_by(|a, b| b.name.cmp(&a.name));
                            self.sorted_by = CurrentSortedBy::Name(Direction::Descending);
                        } else {
                            self.cards.sort_by(|a, b| a.name.cmp(&b.name));
                            self.sorted_by = CurrentSortedBy::Name(Direction::Ascending);
                        }
                    }
                    Category::Quantity => todo!(),
                    Category::Foil => todo!(),
                    Category::Goatbots => todo!(),
                    Category::Scryfall => todo!(),
                    Category::Set => todo!(),
                    Category::Rarity => todo!(),
                }
                self.draw_cards();
            }
        }
    }

    pub fn set_cards(&mut self, cards: Vec<MtgoCard>) {
        self.cards = cards;
        self.draw_cards();
    }

    fn draw_cards(&mut self) {
        if self.cards.is_empty() {
            return;
        }
        // Don't run this gui on some platform with usize < u32 if you're gonna make a huge table
        if self.cards.len() > self.table.row_count() as usize {
            for _ in 0..(self.cards.len() - self.table.row_count() as usize) {
                self.table.append_empty_row("");
            }
        }
        // Fill all the rows with cards data
        for (i, c) in self.cards.iter().enumerate() {
            let row_idx = i as i32;
            self.table.set_cell_value(row_idx, 0, &c.name);
            self.table
                .set_cell_value(row_idx, 1, &c.quantity.to_string());
            self.table
                .set_cell_value(row_idx, 2, if c.foil { "Yes" } else { "No" });
            self.table
                .set_cell_value(row_idx, 3, &format!("{:8.3}", c.goatbots_price));
            self.table.set_cell_value(row_idx, 4, &{
                if let Some(p) = c.scryfall_price {
                    p.to_string()
                } else {
                    "N/A".into()
                }
            });
            self.table.set_cell_value(row_idx, 5, &c.set);
            self.table.set_cell_value(row_idx, 6, &c.rarity);
        }
    }
}

impl Default for CollectionTable {
    fn default() -> Self {
        Self::new(790, 590)
    }
}
