package mtgogetter

import (
	"encoding/json"
	"io"
	"os"
)

type ScryfallCard struct {
	Mtgo_id      int32    `json:"mtgo_id"`
	Mtgo_foil_id int32    `json:"mtgo_foil_id"`
	Name         string   `json:"name"`
	Released_at  string   `json:"released_at"`
	Rarity       string   `json:"rarity"`
	Prices       struct { // All nullable (deserialized as empty string if null)
		Usd      string `json:"usd"`
		Usd_foil string `json:"usd_foil"`
		Eur      string `json:"eur"`
		Eur_foil string `json:"eur_foil"`
		Tix      string `json:"tix"`
	} `json:"prices"`
}

func ScryfallCardsFromFile(fname string) ([]ScryfallCard, error) {
	// Read file to bytes
	file_data, err := os.ReadFile(fname)
	if err != nil {
		return nil, err
	}
	// Unmarshal JSON
	scryfall_cards, err := ScryfallCardsFromJsonBytes(file_data)
	if err != nil {
		return nil, err
	}

	return scryfall_cards, nil
}

func ScryfallCardsFromJsonBytes(byteSlice []byte) ([]ScryfallCard, error) {
	var bulk_data []ScryfallCard
	if err := json.Unmarshal(byteSlice, &bulk_data); err != nil {
		return nil, err
	}

	// Remove cards with mtgo_id == 0 (cards that are not available on MTGO)
	prealloc_size := (len(bulk_data) / 2) + 1

	bulk_data_mtgo := FilterPrealloc(bulk_data, func(c ScryfallCard) bool {
		return c.Mtgo_id != 0
	}, prealloc_size)

	return bulk_data_mtgo, nil
}

// ScryfallCardsFromJsonStream takes a json.Decoder and returns a slice of ScryfallCard structs
// Skips cards with mtgo_id == 0 (cards that are not available on MTGO)
func ScryfallCardsFromJsonStream(decoder *json.Decoder) ([]ScryfallCard, error) {
	var bulk_data []ScryfallCard

	for {
		var card ScryfallCard
		if err := decoder.Decode(&card); err == io.EOF {
			break
		} else if err != nil {
			return nil, err
		}

		// Check if the mtgo_id is non-zero before appending
		if card.Mtgo_id != 0 {
			bulk_data = append(bulk_data, card)
		}
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
