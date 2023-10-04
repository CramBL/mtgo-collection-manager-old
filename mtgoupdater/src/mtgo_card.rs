use serde_derive::{Deserialize, Serialize};

/// This is the struct that represents a card in the MTGO collection.
///
/// It is not the same as a paper card which can have additional fields such as released_at and prices in USD and EUR.
#[derive(Debug, Clone, PartialEq, Default, Serialize, Deserialize)]
pub struct MtgoCard<'s> {
    pub id: u32,
    pub name: &'s str,
    pub set: &'s str,
    pub rarity: &'s str, // TODO: make enum
    pub quantity: u32,
    pub goatbots_price: f32,
    pub scryfall_price: Option<f32>,
}

#[cfg(test)]
mod tests {
    use super::*;
    use pretty_assertions::assert_eq;

    #[test]
    fn test_serialize() {
        let c = MtgoCard::default();

        let serialized = serde_json::to_string_pretty(&c).unwrap();
        println!("serialized = {serialized}");

        let deserialized: MtgoCard = serde_json::from_str(&serialized).unwrap();
        assert_eq!(c, deserialized);
    }

    #[test]
    fn test_deserialize_json_string_vector() {
        let json_vec_str = r#"[
         {
            "id": 1,
            "quantity": 391,
            "name": "Event Ticket",
            "set": "",
            "rarity": "",
            "foil": false,
            "goatbots_price": 0
         },
         {
            "id": 235,
            "quantity": 1,
            "name": "Swamp",
            "set": "PRM",
            "rarity": "Common",
            "foil": false,
            "goatbots_price": 0.002,
            "scryfall_price": 0.05
         }
      ]"#;

        let deserialized: Vec<MtgoCard> = serde_json::from_str(json_vec_str).unwrap();
        assert_eq!(deserialized.len(), 2);
        assert_eq!(deserialized[0].id, 1);
        assert_eq!(deserialized[0].quantity, 391);
        assert_eq!(deserialized[0].name, "Event Ticket");
        assert_eq!(deserialized[0].set, "");
        assert_eq!(deserialized[0].rarity, "");
        assert_eq!(deserialized[0].goatbots_price, 0.0);
        assert_eq!(deserialized[0].scryfall_price, None);
    }
}
