use serde_derive::{Deserialize, Serialize};

#[derive(Debug, Clone, PartialEq, Default, Serialize, Deserialize)]
pub struct Card {
    mtgo_id: u32,
    name: Box<str>,
    set: Box<str>,
    rarity: Box<str>,      // TODO: make enum
    released_at: Box<str>, // TODO: make refactored date type?
    quantity: u32,
    goatbots_price: f32,
    scryfall_prices: ScryfallPrices,
}

#[derive(Debug, Clone, PartialEq, Default, Serialize, Deserialize)]
struct ScryfallPrices {
    tix: f32,
    usd: f32,
    usd_foil: f32,
    eur: f32,
    eur_foil: f32,
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_serialize() {
        let c = Card::default();

        let serialized = serde_json::to_string_pretty(&c).unwrap();
        println!("serialized = {serialized}");

        let deserialized: Card = serde_json::from_str(&serialized).unwrap();
        assert_eq!(c, deserialized);
    }

    #[test]
    fn test_deserialize_json_string_vector() {
        let json_vec_str = r#"[
            {
               "id": "1",
               "quantity": "453",
               "name": "Event Ticket",
               "set": "",
               "rarity": "",
               "foil": false,
               "goatbots_price": 0,
               "scryfall_price": 0
            },
            {
               "id": "235",
               "quantity": "1",
               "name": "Swamp",
               "set": "",
               "rarity": "",
               "foil": false,
               "goatbots_price": 0,
               "scryfall_price": 0
            },
            {
               "id": "31745",
               "quantity": "1",
               "name": "Noble Hierarch",
               "set": "CON",
               "rarity": "Rare",
               "foil": false,
               "goatbots_price": 0.37,
               "scryfall_price": 0
            },
            {
               "id": "53155",
               "quantity": "1",
               "name": "Black Lotus",
               "set": "",
               "rarity": "",
               "foil": false,
               "goatbots_price": 0,
               "scryfall_price": 0
            },
            {
               "id": "110465",
               "quantity": "1",
               "name": "Tranquil Cove",
               "set": "",
               "rarity": "",
               "foil": false,
               "goatbots_price": 0,
               "scryfall_price": 0
            }
         ]"#;
    }
}
