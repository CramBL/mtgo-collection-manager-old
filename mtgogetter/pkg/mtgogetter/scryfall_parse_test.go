package mtgogetter_test

import (
	"log"
	"testing"

	. "github.com/CramBL/mtgo-collection-manager/mtgogetter/pkg/mtgogetter"
)

func TestScryfallJsonParse(t *testing.T) {
	f_scryfall_json := "../../../test/test-data/scryfall/default-cards-small-5cards.json"
    bulk_data := ReadBulkData(f_scryfall_json)

	log.Println("Got", len(bulk_data), "cards")
	if len(bulk_data) != 5 {
		t.Errorf("Expected 5 cards got %d", len(bulk_data))
	}

	first_card := &bulk_data[0]

	if first_card.Mtgo_id != 25527 {
		t.Errorf("Expected 25527 got %d", first_card.Mtgo_id)
	}

	if first_card.Mtgo_foil_id != 25528 {
		t.Errorf("Expected 25528 got %d", first_card.Mtgo_foil_id)
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

	if second_card.Mtgo_foil_id != 31746 {
		t.Errorf("Expected 31746 got %d", second_card.Mtgo_foil_id)
	}

}