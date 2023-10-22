package mtgogetter_test

import (
	"log"
	"os"
	"testing"
	"time"

	. "github.com/CramBL/mtgo-collection-manager/mtgogetter/pkg/mtgogetter"
)

func TestScryfallJsonDeserialize(t *testing.T) {
	f_scryfall_json := "../../../test/test-data/scryfall/default-cards-small-5cards.json"
	bulk_data, err := ScryfallCardsFromFile(f_scryfall_json)
	if err != nil {
		t.Errorf("Error when parsing Scryfall JSON: %s", err)
	}

	log.Println("Got", len(bulk_data), "cards")
	if len(bulk_data) != 5 {
		t.Errorf("Expected 5 cards got %d", len(bulk_data))
	}

	first_card := &bulk_data[0]

	if first_card.Mtgo_id != 25527 {
		t.Errorf("Expected 25527 got %d", first_card.Mtgo_id)
	}

	if first_card.Name != "Fury Sliver" {
		t.Errorf("Expected Fury Sliver got %s", bulk_data[0].Name)
	}

	if first_card.Prices.Usd != "0.47" {
		t.Errorf("Expected 0.47 got %s", bulk_data[0].Prices.Usd)
	}

	second_card := &bulk_data[1]

	if second_card.Mtgo_id != 31745 {
		t.Errorf("Expected 31745 got %d", second_card.Mtgo_id)
	}

	// Check third cards prices
	// Black Lotus from Vintage Masters (only on MTGO so other prices are null)
	third_lotus := &bulk_data[2]

	if third_lotus.Mtgo_id != 53155 {
		t.Errorf("Expected 53155 got %d", third_lotus.Mtgo_id)
	}

	if third_lotus.Prices.Usd != "" { // null in JSON
		t.Errorf("Expected empty string, got %s", third_lotus.Prices.Usd)
	}

	if third_lotus.Prices.Tix != "13.51" {
		t.Errorf("Expected 13.51 got %s", third_lotus.Prices.Tix)
	}
}

func TestScryfallJsonSerialize(t *testing.T) {
	f_scryfall_json := "../../../test/test-data/scryfall/default-cards-small-5cards.json"
	bulk_data, err := ScryfallCardsFromFile(f_scryfall_json)
	if err != nil {
		t.Errorf("Error when parsing Scryfall JSON: %s", err)
	}

	serialized_bulk_data, err := SerializeScryfallCards(bulk_data)
	if err != nil {
		t.Errorf("Error when serializing Scryfall JSON: %s", err)
	}

	// Deserialize again and check that it matches the first deserialization
	deserialized_bulk_data, err := ScryfallCardsFromJsonBytes(serialized_bulk_data)
	if err != nil {
		t.Errorf("Error when deserializing Scryfall JSON: %s", err)
	}

	if len(deserialized_bulk_data) != len(bulk_data) {
		t.Errorf("Expected %d cards got %d", len(bulk_data), len(deserialized_bulk_data))
	}

	for i := range deserialized_bulk_data {

		if deserialized_bulk_data[i] != bulk_data[i] {
			t.Errorf("Expected %v got %v", bulk_data[i], deserialized_bulk_data[i])
		}

	}

}

func TestScryfallJson_Deserialize_50cards(t *testing.T) {
	// Contains 50 cards with mtgo_id (cards that exist on MTGO)
	// But contains 87 card objects in total
	// the cards that don't exist on MTGO have mtgo_id == 0 and should not be deserialized
	f_scryfall_json := "../../../test/test-data/scryfall/default-cards-small-87objs-50cards.json"

	bulk_data, err := ScryfallCardsFromFile(f_scryfall_json)
	if err != nil {
		t.Fatalf("Error when parsing Scryfall JSON: %s", err)
	}

	if len(bulk_data) != 50 {
		t.Errorf("Expected 50 cards got %d", len(bulk_data))
	}

	if err != nil {
		t.Errorf("Error when parsing Scryfall JSON: %s", err)
	}

	first_card := &bulk_data[0]

	if first_card.Mtgo_id != 25527 {
		t.Errorf("Expected 25527 got %d", first_card.Mtgo_id)
	}

	if first_card.Name != "Fury Sliver" {
		t.Errorf("Expected Fury Sliver got %s", bulk_data[0].Name)
	}

	if first_card.Prices.Usd != "0.47" {
		t.Errorf("Expected 0.47 got %s", bulk_data[0].Prices.Usd)
	}

	second_card := &bulk_data[1]

	if second_card.Mtgo_id != 34586 {
		t.Errorf("Expected 34586 got %d", second_card.Mtgo_id)
	}

	// Check third card
	third_card := &bulk_data[2]

	if third_card.Mtgo_id != 65170 {
		t.Errorf("Expected 65170 got %d", third_card.Mtgo_id)
	}

	if third_card.Prices.Usd != "0.03" {
		t.Errorf("Expected 0.03, got %s", third_card.Prices.Usd)
	}

	if third_card.Prices.Tix != "0.03" {
		t.Errorf("Expected 0.03 got %s", third_card.Prices.Tix)
	}
}

func TestScryfallJson_Deserialize_50cards_streamed(t *testing.T) {
	// Contains 50 cards with mtgo_id (cards that exist on MTGO)
	// But contains 87 card objects in total
	// the cards that don't exist on MTGO have mtgo_id == 0 and should not be deserialized
	f_scryfall_json := "../../../test/test-data/scryfall/default-cards-small-87objs-50cards.json"

	bulk_data, err := ScryfallCardsFromFileStreamed(f_scryfall_json)

	if err != nil {
		t.Fatalf("Error when parsing Scryfall JSON: %s", err)
	}

	if len(bulk_data) != 50 {
		t.Fatalf("Expected 50 cards got %d", len(bulk_data))
	}

	if err != nil {
		t.Errorf("Error when parsing Scryfall JSON: %s", err)
	}

	first_card := &bulk_data[0]

	if first_card.Mtgo_id != 25527 {
		t.Errorf("Expected 25527 got %d", first_card.Mtgo_id)
	}

	if first_card.Name != "Fury Sliver" {
		t.Errorf("Expected Fury Sliver got %s", bulk_data[0].Name)
	}

	if first_card.Prices.Usd != "0.47" {
		t.Errorf("Expected 0.47 got %s", bulk_data[0].Prices.Usd)
	}

	second_card := &bulk_data[1]

	if second_card.Mtgo_id != 34586 {
		t.Errorf("Expected 34586 got %d", second_card.Mtgo_id)
	}

	// Check third card
	third_card := &bulk_data[2]

	if third_card.Mtgo_id != 65170 {
		t.Errorf("Expected 65170 got %d", third_card.Mtgo_id)
	}

	if third_card.Prices.Usd != "0.03" {
		t.Errorf("Expected 0.03, got %s", third_card.Prices.Usd)
	}

	if third_card.Prices.Tix != "0.03" {
		t.Errorf("Expected 0.03 got %s", third_card.Prices.Tix)
	}
}

func TestNewestAnnouncedScryfallSetFromJsonBytes(t *testing.T) {
	var scryfallSetJson = []byte(`
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
	    }
	  ]
	}`)

	mostRecentSet, err := NewestAnnouncedScryfallSetFromJsonBytes(scryfallSetJson)
	if err != nil {
		t.Errorf("Failed decoding scryfall set json: %s", err)
	}

	expectSetName := "Ravnica Remastered"
	if mostRecentSet.Name != expectSetName {
		t.Errorf("Expected most recent set name to be %s, got: %s", expectSetName, mostRecentSet.Name)
	}

	expectReleasedAt := "2024-03-01"
	if mostRecentSet.Released_at != expectReleasedAt {
		t.Errorf("Expected most recent set to be released at %s, got %s", expectReleasedAt, mostRecentSet.Released_at)
	}

}

func TestNextReleasedScryfallSetFromJsonBytes_simple(t *testing.T) {
	// Read the JSON file into a byte slice
	scryfallSetJson, err := os.ReadFile("../../../test/test-data/scryfall/sets-small-16sets.json")
	if err != nil {
		t.Errorf("Error reading file: %s", err)
	}

	targetTime := time.Date(2023, time.October, 14, 0, 0, 0, 0, time.UTC)

	mostRecentSet, err := NextReleasedScryfallSetFromJsonBytes(scryfallSetJson, targetTime)
	if err != nil {
		t.Errorf("Failed decoding scryfall set json: %s", err)
	}

	expectSetName := "Lost Caverns of Ixalan Commander"
	if mostRecentSet.Name != expectSetName {
		t.Errorf("Expected most recent set name to be %s, got: %s", expectSetName, mostRecentSet.Name)
	}

	expectReleasedAt := "2023-11-17"
	if mostRecentSet.Released_at != expectReleasedAt {
		t.Errorf("Expected most recent set to be released at %s, got %s", expectReleasedAt, mostRecentSet.Released_at)
	}
}

// Tests the scenario where the date is exactly equal to a set's release
// it should be the same behaviour as if the date was just before that set's release
func TestNextReleasedScryfallSetFromJsonBytes_exactDate(t *testing.T) {
	// Read the JSON file into a byte slice
	scryfallSetJson, err := os.ReadFile("../../../test/test-data/scryfall/sets-small-16sets.json")
	if err != nil {
		t.Errorf("Error reading file: %s", err)
	}

	targetTime := time.Date(2023, time.November, 17, 0, 0, 0, 0, time.UTC)

	mostRecentSet, err := NextReleasedScryfallSetFromJsonBytes(scryfallSetJson, targetTime)
	if err != nil {
		t.Errorf("Failed decoding scryfall set json: %s", err)
	}

	expectSetName := "Lost Caverns of Ixalan Commander"
	if mostRecentSet.Name != expectSetName {
		t.Errorf("Expected most recent set name to be %s, got: %s", expectSetName, mostRecentSet.Name)
	}

	expectReleasedAt := "2023-11-17"
	if mostRecentSet.Released_at != expectReleasedAt {
		t.Errorf("Expected most recent set to be released at %s, got %s", expectReleasedAt, mostRecentSet.Released_at)
	}
}

// Tests where the target date is later than the latest set release, meaning we won't find a set that will be released in the future
// this should never happen and should error
func TestNextReleasedScryfallSetFromJsonBytes_laterDateErrors(t *testing.T) {
	// Read the JSON file into a byte slice
	scryfallSetJson, err := os.ReadFile("../../../test/test-data/scryfall/sets-small-16sets.json")
	if err != nil {
		t.Errorf("Error reading file: %s", err)
	}

	targetTime := time.Date(2024, time.November, 17, 0, 0, 0, 0, time.UTC)

	mostRecentSet, err := NextReleasedScryfallSetFromJsonBytes(scryfallSetJson, targetTime)
	if err == nil {
		t.Errorf("Expected failure when target date is later than the latest release, instead got set: %s ", mostRecentSet.Name)
	}

}
