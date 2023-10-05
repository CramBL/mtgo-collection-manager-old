use fltk::enums::Color;
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

pub(super) struct CollectionTable {
    pub(super) table: SmartTable,
    pub(super) cards: Vec<MtgoCard>,
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
        }
    }

    pub fn handle_ev(&mut self, ev: CtMessage) {
        match ev {
            CtMessage::SortBy(cat) => {
                println!("sort by {:?}", cat);
                match cat {
                    Category::Name => todo!(),
                    Category::Quantity => todo!(),
                    Category::Foil => todo!(),
                    Category::Goatbots => todo!(),
                    Category::Scryfall => todo!(),
                    Category::Set => todo!(),
                    Category::Rarity => todo!(),
                }
            }
        }
    }

    pub fn set_cards(&mut self, cards: Vec<MtgoCard>) {
        self.cards = cards;
        self.cards.iter().for_each(|c| {
            self.table.append_row(
                "",
                &[
                    &c.name,
                    &c.quantity.to_string(),
                    if c.foil { "Yes" } else { "No" },
                    &format!("{:8.3}", c.goatbots_price),
                    &{
                        if let Some(p) = c.scryfall_price {
                            p.to_string()
                        } else {
                            "N/A".into()
                        }
                    },
                    &c.set,
                    &c.rarity,
                ],
            );
        });
    }
}

impl Default for CollectionTable {
    fn default() -> Self {
        Self::new(790, 590)
    }
}
