package mtgogetter_test

import (
	"bytes"
	"fmt"
	"strings"
	"testing"
	"time"

	"github.com/BurntSushi/toml"
	. "github.com/CramBL/mtgo-collection-manager/mtgogetter/pkg/mtgogetter"
)



func TestStateLogDeserialize(t *testing.T) {
	// Make default statelog and convert it to TOML and then to a string
	state_log_default := NewStateLog()

	var state_log_buf bytes.Buffer
	if err := toml.NewEncoder(&state_log_buf).Encode(state_log_default); err != nil {
		t.Errorf("Error when encoding default StateLog to TOML: %s", err)
	}

	state_log_str := state_log_buf.String()
	fmt.Printf("state_log_str:\n%s\n", state_log_str)

	// Deserialize the TOML into a StateLog struct
	var state_log StateLog

	if _, err := toml.Decode(state_log_str, &state_log); err != nil {
		t.Errorf("Error when parsing StateLog TOML: %s", err)
	}

	if state_log.Title != "log for MTGO Getter state, such as updated_at timestamps" {
		t.Errorf("Expected 'log for MTGO Getter state, such as updated_at timestamps' got %s", state_log.Title)
	}

	if !strings.Contains(state_log.Goatbots.Updated_at.String(), "1970-01-01 01:00:00") {
		t.Errorf("Expected updated_at to contain '1970-01-01 01:00:00' got %s", state_log.Goatbots.Updated_at.String())
	}

	if state_log.Scryfall.Updated_at.Day() != 1 {
		t.Errorf("Expected 1 got %d", state_log.Scryfall.Updated_at.Day())
	}
	if state_log.Scryfall.Updated_at.Hour() != 1 {
		t.Errorf("Expected 1 got %d", state_log.Scryfall.Updated_at.Hour())
	}

	if !state_log.Scryfall.Updated_at.Equal(state_log.Goatbots.Updated_at) {
		t.Errorf("Expected Goatbots and Scryfall updated_at to be the same")
	}
	if !state_log.Goatbots.Updated_at.Equal(state_log.Goatbots.Prices_updated_at) {
		t.Errorf("Expected Goatbots and Goatbots prices_updated_at to be the same, but got \n%s\n!=\n%s\n", state_log.Goatbots.Updated_at.String(), state_log.Goatbots.Prices_updated_at.String())
	}

	// Check that the updated_at timestamps are equal to Unix epoch
	if !state_log.Scryfall.Updated_at.Equal(time.Unix(0, 0)) {
		t.Errorf("Expected the timestamps to be equal to the unix epoch, got %d", state_log.Scryfall.Updated_at.Unix())
	}
}