use mtgoupdater::mtgo_card::MtgoCard;

use super::util::{MultiValueStat, UniqueTotal};

/// Container for collection stats
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
    /// Create a new empty [CollectionStats] container
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

    fn calc_total_cards(&mut self, cards: &[MtgoCard]) {
        let (unique_count, quantity_count) = cards
            .iter()
            .fold((0, 0), |acc, card| (acc.0 + 1, acc.1 + card.quantity));

        self.set_total_cards(unique_count, quantity_count as usize);
    }

    fn calc_most_expensive_item(&mut self, cards: &[MtgoCard]) {
        let gb_most_expensive = cards
            .iter()
            .max_by(|a, b| a.goatbots_price.partial_cmp(&b.goatbots_price).unwrap())
            .unwrap_or_else(|| panic!("No cards in collection!"));
        let scryfall_most_expensive = cards
            .iter()
            .max_by(|a, b| {
                a.scryfall_price
                    .partial_cmp(&b.scryfall_price)
                    .unwrap_or(std::cmp::Ordering::Equal)
            })
            .unwrap_or_else(|| panic!("No cards in collection!"));

        let description = if gb_most_expensive.goatbots_price
            > scryfall_most_expensive.scryfall_price.unwrap_or(0.)
        {
            format!(
                "{name} ({price} tix @Goatbots)",
                name = gb_most_expensive.name.as_ref(),
                price = gb_most_expensive.goatbots_price
            )
        } else {
            format!(
                "{name} ({price} tix @Cardhoarder)",
                name = scryfall_most_expensive.name.as_ref(),
                price = scryfall_most_expensive.scryfall_price.unwrap()
            )
        };
        self.set_most_expensive_item(&description);
    }

    fn calc_total_value(&mut self, cards: &[MtgoCard]) {
        let total_gb_value = cards
            .iter()
            .map(|card| card.goatbots_price as f64 * card.quantity as f64)
            .sum::<f64>();
        let total_scryfall_value = cards.iter().fold(0., |acc, card| {
            acc + card
                .scryfall_price
                .map_or(0., |price| price as f64 * card.quantity as f64)
        });

        self.total_value = Some(MultiValueStat::new(
            "Total value".to_string(),
            if total_gb_value > total_scryfall_value {
                vec![
                    format!("@C2 {:.2} tix @Goatbots", total_gb_value),
                    format!("@C3 {:.2} tix @Cardhoarder", total_scryfall_value),
                ]
            } else {
                vec![
                    format!("@C3 {:.2} tix @Goatbots", total_gb_value),
                    format!("@C2 {:.2} tix @Cardhoarder", total_scryfall_value),
                ]
            },
        ));
    }

    fn calc_cards_under_tix(price: f32, cards: &[MtgoCard]) -> UniqueTotal {
        let (unique_count, quantity_count) = cards.iter().fold((0, 0), |acc, card| {
            if card.goatbots_price < price {
                (acc.0 + 1, acc.1 + card.quantity)
            } else {
                acc
            }
        });

        UniqueTotal::new(unique_count, quantity_count as usize)
    }

    fn calc_cards_over_tix(price: f32, cards: &[MtgoCard]) -> UniqueTotal {
        let (unique_count, quantity_count) = cards.iter().fold((0, 0), |acc, card| {
            if card.goatbots_price > price {
                (acc.0 + 1, acc.1 + card.quantity)
            } else {
                acc
            }
        });
        UniqueTotal::new(unique_count, quantity_count as usize)
    }

    /// Create a new [CollectionStats] from a list of cards
    ///
    /// # Arguments
    ///
    /// * `cards` - A borrowed slice of cards to create stats from
    ///
    /// # Returns
    ///
    /// A new [CollectionStats] container
    pub fn from_cards(cards: &[MtgoCard]) -> Self {
        let mut stats = Self::new();

        stats.calc_total_cards(cards);
        stats.calc_most_expensive_item(cards);
        stats.calc_total_value(cards);
        stats.cards_under_a_tenth_tix = Self::calc_cards_under_tix(0.1, cards);
        stats.cards_over_5_tix = Self::calc_cards_over_tix(5.0, cards);
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
