package mtgogetter

import (
	"encoding/json"
	"fmt"
	"os"
)

/*
	{
	  "object": "list",
	  "has_more": false,
	  "data": [
	    {
	      "object": "set",
	      "id": "fed2c8cd-ab92-44f6-808a-41e7809bcfe2",
	      "code": "rvr",
	      "tcgplayer_id": 23319,
	      "name": "Ravnica Remastered",
	      "uri": "https://api.scryfall.com/sets/fed2c8cd-ab92-44f6-808a-41e7809bcfe2",
	      "scryfall_uri": "https://scryfall.com/sets/rvr",
	      "search_uri": "https://api.scryfall.com/cards/search?include_extras=true&include_variations=true&order=set&q=e%3Arvr&unique=prints",
	      "released_at": "2024-03-01",
	      "set_type": "masters",
	      "card_count": 50,
	      "digital": false,
	      "nonfoil_only": false,
	      "foil_only": false,
	      "icon_svg_uri": "https://svgs.scryfall.io/sets/rvr.svg?1696824000"
	    },
		...
	}
*/
type ScryfallSetBlob struct {
	Object   string        `json:"object"`
	Has_more bool          `json:"has_more"`
	Data     []ScryfallSet `json:"data"`
}

type ScryfallSet struct {
	Name        string `json:"name"`
	Released_at string `json:"released_at"`
}

type ScryfallCard struct {
	Mtgo_id     int32    `json:"mtgo_id"`
	Name        string   `json:"name"`
	Released_at string   `json:"released_at"`
	Rarity      string   `json:"rarity"`
	Prices      struct { // All nullable (deserialized as empty string if null)
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

// Streamdecoder from a file
func ScryfallCardsFromFileStreamed(fname string) ([]ScryfallCard, error) {
	fd, err := os.Open(fname)
	if err != nil {
		return nil, err
	}
	defer fd.Close()

	decoder := json.NewDecoder(fd)

	bulk_data, err := ScryfallCardsFromJsonStream(decoder)
	if err != nil {
		return nil, err
	}

	return bulk_data, nil
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

func MostRecentScryfallSetFromJsonBytes(byteSlice []byte) (*ScryfallSet, error) {
	var blob ScryfallSetBlob
	if err := json.Unmarshal(byteSlice, &blob); err != nil {
		return nil, err
	}

	if len(blob.Data) == 0 {
		return nil, fmt.Errorf("no data found")
	}

	return &blob.Data[0], nil
}

//	takes a json.Decoder and returns a slice of ScryfallCard structs
//
// Skips cards with mtgo_id == 0 (cards that are not available on MTGO)
func ScryfallCardsFromJsonStream(decoder *json.Decoder) ([]ScryfallCard, error) {
	var bulk_data []ScryfallCard

	err := decodeScryfallMtgoCardsJsonArray(decoder, &bulk_data)
	if err != nil {
		return nil, err
	}

	return bulk_data, nil
}

// Takes a json.Decoder and returns a slice of ScryfallCard structs
// Skips cards with mtgo_id == 0 (cards that are not available on MTGO)
// Preallocates a slice of ScryfallCard structs with the given size
func ScryfallCardsFromStreamPrealloc(decoder *json.Decoder, prealloc int) ([]ScryfallCard, error) {
	bulk_data := make([]ScryfallCard, 0, prealloc)

	err := decodeScryfallMtgoCardsJsonArray(decoder, &bulk_data)
	if err != nil {
		return nil, err
	}

	return bulk_data, nil
}

func decodeScryfallMtgoCardsJsonArray(decoder *json.Decoder, buf *[]ScryfallCard) error {
	// Expect the start of an array
	if _, err := decoder.Token(); err != nil {
		return err
	}

	for decoder.More() {
		var card ScryfallCard
		if err := decoder.Decode(&card); err != nil {
			return nil
		}

		// Check if the mtgo_id is non-zero before appending
		if card.Mtgo_id != 0 {
			*buf = append(*buf, card)
		}
	}

	// Expect the end of the array
	if _, err := decoder.Token(); err != nil {
		return err
	}
	return nil
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
