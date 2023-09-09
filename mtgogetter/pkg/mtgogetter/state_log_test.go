package mtgogetter_test

import (
	"testing"

	"github.com/BurntSushi/toml"
	. "github.com/CramBL/mtgo-collection-manager/mtgogetter/pkg/mtgogetter"
)

var state_log_str string = `title = "log for mtgogetter state, such as updated_at timestamps"
	[goatbots]
	updated_at = "1970-01-01T00:00:00Z"

	[scryfall]
	updated_at = "1970-01-01T00:00:00Z"`

func TestStateLogDeserialize(t *testing.T) {
	var state_log StateLog

	if _, err := toml.Decode(state_log_str, &state_log); err != nil {
		t.Errorf("Error when parsing StateLog TOML: %s", err)
	}

	if state_log.Title != "log for mtgogetter state, such as updated_at timestamps" {
		t.Errorf("Expected 'log for mtgogetter state, such as updated_at timestamps' got %s", state_log.Title)
	}

	if state_log.Goatbots.Updated_at.String() != "1970-01-01 00:00:00 +0000 UTC" {
		t.Errorf("Expected '1970-01-01T00:00:00Z' got %s", state_log.Goatbots.Updated_at.String())
	}

	if state_log.Scryfall.Updated_at.Day() != 1 {
		t.Errorf("Expected 1 got %d", state_log.Scryfall.Updated_at.Day())
	}
}