package mtgogetter

import (
	"encoding/json"
	"os"
)

type ScryfallCard struct{
    Mtgo_id int32 `json:"mtgo_id"`
	Mtgo_foil_id int32 `json:"mtgo_foil_id"`
	Name string `json:"name"`
	Released_at string `json:"released_at"`
	Rarity string `json:"rarity"`
	Prices struct { // All nullable (deserialized as empty string if null)
		Usd string `json:"usd"`
		Usd_foil string `json:"usd_foil"`
		Eur string `json:"eur"`
		Eur_foil string `json:"eur_foil"`
		Tix string `json:"tix"`
	} `json:"prices"`
}

func ScryfallCardsFromFile(fname string	) ([]ScryfallCard, error) {
	// Read file to bytes
	file_data, err := os.ReadFile(fname)
	if err != nil {
		return nil, err
	}
	// Unmarshal JSON
	scryfall_cards, err := DeserializeScryfallCards(file_data)
	if err != nil {
		return nil, err
	}

	return scryfall_cards, nil
}

func DeserializeScryfallCards(byteSlice []byte) ([]ScryfallCard, error) {
	var bulk_data []ScryfallCard
	if err := json.Unmarshal(byteSlice, &bulk_data); err != nil {
		return nil, err
	}
	return bulk_data, nil
}

func SerializeScryfallCards(scryfall_cards []ScryfallCard) ([]byte, error) {
	return json.Marshal(scryfall_cards)
}

func ScryfallCardsToDisk(scryfall_cards []ScryfallCard, fname string) error {
	scryfall_cards_bytes, err := SerializeScryfallCards(scryfall_cards)
	if err != nil {
		return err
	}
	return os.WriteFile(fname, scryfall_cards_bytes, 0644)
}