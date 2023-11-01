use mtgoupdater::mtgo_card::MtgoCard;

use super::util::{MultiValueStat, UniqueTotal};

#[derive(Debug, Clone)]
pub struct CollectionStats {
    file_from: String,
    total_cards: UniqueTotal,
    total_value: Option<MultiValueStat>,
    most_expensive_item: String,
    cards_under_a_tenth_tix: UniqueTotal,
    cards_over_5_tix: UniqueTotal,
}

impl CollectionStats {
    pub fn new() -> Self {
        Self {
            file_from: String::new(),
            total_value: None,
            total_cards: UniqueTotal::default(),
            most_expensive_item: String::new(),
            cards_under_a_tenth_tix: UniqueTotal::default(),
            cards_over_5_tix: UniqueTotal::default(),
        }
    }

    pub fn from_cards(cards: &[MtgoCard]) -> Self {
        let mut stats = Self::new();

        let mut total_cards_unique = 0;
        let mut total_cards_quantity = 0;

        let mut most_expensive_item = 0.;

        let mut cards_under_a_tenth_tix_unique = 0;
        let mut cards_under_a_tenth_tix_quantity = 0;

        let mut cards_over_5_tix_unique = 0;
        let mut cards_over_5_tix_quantity = 0;

        for card in cards {
            total_cards_unique += 1;
            total_cards_quantity += card.quantity;

            if card.goatbots_price > most_expensive_item {
                most_expensive_item = card.goatbots_price;
                let description = format!(
                    "{} ({} tix @Goatbots)",
                    card.name.as_ref(),
                    card.goatbots_price
                );
                stats.set_most_expensive_item(&description);
            }

            if card
                .scryfall_price
                .is_some_and(|price| price > most_expensive_item)
            {
                most_expensive_item = card.scryfall_price.unwrap();
                let description = format!(
                    "{} ({} tix @Cardhoarder)",
                    card.name.as_ref(),
                    card.scryfall_price.unwrap()
                );
                stats.set_most_expensive_item(&description);
            }

            if card.goatbots_price < 0.1 {
                cards_under_a_tenth_tix_unique += 1;
                cards_under_a_tenth_tix_quantity += card.quantity;
            }

            if card.goatbots_price > 5. {
                cards_over_5_tix_unique += 1;
                cards_over_5_tix_quantity += card.quantity;
            }
        }

        let total_gb_value = cards
            .iter()
            .map(|card| card.goatbots_price as f64 * card.quantity as f64)
            .sum::<f64>();
        let total_scryfall_value = cards.iter().fold(0., |acc, card| {
            acc + card
                .scryfall_price
                .map_or(0., |price| price as f64 * card.quantity as f64)
        });

        stats.total_value = Some(MultiValueStat::new(
            "Total value".to_string(),
            vec![
                format!("{:.2} tix @Goatbots", total_gb_value),
                format!("{:.2} tix @Cardhoarder", total_scryfall_value),
            ],
        ));

        stats.set_total_cards(total_cards_unique, total_cards_quantity as usize);
        stats.set_cards_under_a_tenth_tix(
            cards_under_a_tenth_tix_unique,
            cards_under_a_tenth_tix_quantity as usize,
        );
        stats.set_cards_over_5_tix(cards_over_5_tix_unique, cards_over_5_tix_quantity as usize);

        stats
    }

    pub fn set_file_from(&mut self, file_from: &str) {
        self.file_from = file_from.to_string();
    }

    pub fn set_total_cards(&mut self, total_unique_cards: usize, total_card_quantity: usize) {
        self.total_cards = UniqueTotal::new(total_unique_cards, total_card_quantity);
    }

    pub fn set_most_expensive_item(&mut self, most_expensive_item: &str) {
        self.most_expensive_item = most_expensive_item.to_string();
    }

    pub fn set_cards_under_a_tenth_tix(
        &mut self,
        cards_under_tenth_tix_unique: usize,
        cards_under_tenth_tix_quantity: usize,
    ) {
        self.cards_under_a_tenth_tix =
            UniqueTotal::new(cards_under_tenth_tix_unique, cards_under_tenth_tix_quantity);
    }

    pub fn set_cards_over_5_tix(
        &mut self,
        cards_over_5_tix_unique: usize,
        cards_over_5_tix_quantity: usize,
    ) {
        self.cards_over_5_tix =
            UniqueTotal::new(cards_over_5_tix_unique, cards_over_5_tix_quantity);
    }

    pub fn file_from(&self) -> &str {
        &self.file_from
    }

    pub fn total_cards(&self) -> UniqueTotal {
        self.total_cards
    }

    pub fn most_expensive_item(&self) -> &str {
        &self.most_expensive_item
    }

    pub fn cards_under_a_tenth_tix(&self) -> UniqueTotal {
        self.cards_under_a_tenth_tix
    }

    pub fn cards_over_5_tix(&self) -> UniqueTotal {
        self.cards_over_5_tix
    }

    pub fn take_total_value(&mut self) -> Option<MultiValueStat> {
        self.total_value.take()
    }
}

impl Default for CollectionStats {
    fn default() -> Self {
        Self::new()
    }
}
