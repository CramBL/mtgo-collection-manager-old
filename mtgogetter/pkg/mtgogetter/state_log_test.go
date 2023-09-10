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

	substr := "1970-01-01 00:00:00"
	if !strings.Contains(state_log.Goatbots.Card_definitions_updated_at.String(), substr) {
		t.Errorf("Expected updated_at to contain %s got %s", substr, state_log.Goatbots.Card_definitions_updated_at.String())
	}

	if state_log.Scryfall.Bulk_data_updated_at.Day() != 1 {
		t.Errorf("Expected 1 got %d", state_log.Scryfall.Bulk_data_updated_at.Day())
	}
	if state_log.Scryfall.Bulk_data_updated_at.Hour() != 0 {
		t.Errorf("Expected hour is 0 got %d", state_log.Scryfall.Bulk_data_updated_at.Hour())
	}

	if !state_log.Scryfall.Bulk_data_updated_at.Equal(state_log.Goatbots.Card_definitions_updated_at) {
		t.Errorf("Expected Goatbots and Scryfall updated_at to be the same")
	}
	if !state_log.Goatbots.Card_definitions_updated_at.Equal(state_log.Goatbots.Prices_updated_at) {
		t.Errorf("Expected Goatbots and Goatbots prices_updated_at to be the same, but got \n%s\n!=\n%s\n", state_log.Goatbots.Card_definitions_updated_at.String(), state_log.Goatbots.Prices_updated_at.String())
	}

	// Check that the updated_at timestamps are equal to Unix epoch
	if !state_log.Scryfall.Bulk_data_updated_at.Equal(time.Unix(0, 0)) {
		t.Errorf("Expected the timestamps to be equal to the unix epoch, got %d", state_log.Scryfall.Bulk_data_updated_at.Unix())
	}
}

func TestStateLogGoatbotsUpdateTime_PriceIsOutdated(t *testing.T) {
	// Test that the PriceUpdateAvailable function works as expected
	state_log := NewStateLog()

	// Set the updated_at timestamps to 3:31 AM UTC (then there should be a new updated prices available)
	utc_now := time.Now().UTC()
	utc_4am := time.Date(utc_now.Year(), utc_now.Month(), utc_now.Day(), 4, 0, 0, 0, time.UTC)
	if utc_now.Before(utc_4am) {
		utc_3_31am_yesterday := time.Date(utc_now.Year(), utc_now.Month(), utc_now.Day()-1, 3, 31, 0, 0, time.UTC)
		state_log.Goatbots.Prices_updated_at = time.Unix(utc_3_31am_yesterday.Unix(), 0)
	} else {
		utc_3_31am := time.Date(utc_now.Year(), utc_now.Month(), utc_now.Day(), 3, 31, 0, 0, time.UTC)
		state_log.Goatbots.Prices_updated_at = time.Unix(utc_3_31am.Unix(), 0)
	}

	fmt.Println(state_log.Goatbots.Prices_updated_at.String())

	// Test that the returns false when the updated_at timestamp is before 4 AM yesterday
	if state_log.Goatbots.IsPriceUpdated() {
		t.Errorf("Expected PriceUpdateAvailable to return true")
	}
}

func TestStateLogGoatbotsUpdateTime_PriceIsUpdated(t *testing.T) {
	// Test that the PriceUpdateAvailable function works as expected
	state_log := NewStateLog()

	// Set the updated_at timestamps to 4:01 am UTC (then it should be up-to-date)
	utc_now := time.Now().UTC()
	utc_4am := time.Date(utc_now.Year(), utc_now.Month(), utc_now.Day(), 4, 0, 0, 0, time.UTC)
	if utc_now.Before(utc_4am) {
		utc_4_01am_yesterday := time.Date(utc_now.Year(), utc_now.Month(), utc_now.Day()-1, 4, 1, 0, 0, time.UTC)
		state_log.Goatbots.Prices_updated_at = time.Unix(utc_4_01am_yesterday.Unix(), 0)
	} else {
		utc_4_01am := time.Date(utc_now.Year(), utc_now.Month(), utc_now.Day(), 4, 1, 0, 0, time.UTC)
		state_log.Goatbots.Prices_updated_at = time.Unix(utc_4_01am.Unix(), 0)
	}

	fmt.Println(state_log.Goatbots.Prices_updated_at.UTC().String())

	// Test that the function returns true when the updated_at timestamp is after 4 AM yesterday
	if !state_log.Goatbots.IsPriceUpdated() {
		t.Errorf("Expected PriceUpdateAvailable to return true for: %s", state_log.Goatbots.Prices_updated_at.UTC().String())
	}
}

func TestStateLogGoatbotsUpdateTime_PriceIsUpdated_local(t *testing.T) {
	// Test that the PriceUpdateAvailable function works as expected
	state_log := NewStateLog()

	// Set the updated_at timestamp to the local time 25 hours ago (then it should be time to update)
	local_now := time.Now().Local()
	local_24_hours_ago := time.Date(local_now.Year(), local_now.Month(), local_now.Day()-1, local_now.Hour(), local_now.Minute(), 0, 0, local_now.Location())
	local_25_hours_ago := local_24_hours_ago.Add(-1 * time.Hour)

	state_log.Goatbots.Prices_updated_at = time.Unix(local_25_hours_ago.Unix(), 0)
	fmt.Println(state_log.Goatbots.Prices_updated_at.String())

	// Test that the function returns true when the updated_at timestamp is after 4 AM yesterday
	if state_log.Goatbots.IsPriceUpdated() {
		t.Errorf("Expected PriceUpdateAvailable to return true for: %s", state_log.Goatbots.Prices_updated_at.String())
	}
}

func TestIsBulkDataUpdated_IsUpdated(t *testing.T) {
	// Test that the PriceUpdateAvailable function works as expected
	state_log := NewStateLog()

	// Set the updated_at timestamps to now (then it should be up-to-date)
	utc_now := time.Now().UTC().Unix()
	state_log.Scryfall.Bulk_data_updated_at = time.Unix(utc_now, 0).UTC()
	fmt.Println("Bulk data updated_at:", state_log.Scryfall.Bulk_data_updated_at.UTC().String())

	// Set the updated_at timestamp `received from the Scryfall API` to 1 hour ago
	utc_1_hour_ago := time.Unix(utc_now-3600, 0).UTC()
	fmt.Println("MOCK Scryfall API timestamp:", utc_1_hour_ago.UTC().String())

	// Test that the function returns true when the updated_at timestamp is after 4 AM yesterday
	if !state_log.Scryfall.IsBulkDataUpdated(utc_1_hour_ago) {
		t.Errorf("Expected PriceUpdateAvailable to return true for: %s", state_log.Scryfall.Bulk_data_updated_at.UTC().String())
	}
}

func TestIsBulkDataUpdated_NotUpdated(t *testing.T) {
	// Test that the PriceUpdateAvailable function works as expected
	state_log := NewStateLog()

	// Set the updated_at timestamps to yesterday (then it should be NOT be up-to-date)
	utc_now := time.Now().UTC().Unix()
	state_log.Scryfall.Bulk_data_updated_at = time.Unix(utc_now - 84600, 0).UTC()
	fmt.Println("Bulk data updated_at:", state_log.Scryfall.Bulk_data_updated_at.UTC().String())

	// Set the updated_at timestamp `received from the Scryfall API` to 30 minutes ago
	utc_30_min_ago := time.Unix(utc_now-300, 0).UTC()
	fmt.Println("MOCK Scryfall API timestamp:", utc_30_min_ago.UTC().String())

	// Test that the function returns true when the updated_at timestamp is after 4 AM yesterday
	if state_log.Scryfall.IsBulkDataUpdated(utc_30_min_ago) {
		t.Errorf("Expected PriceUpdateAvailable to return true for: %s", state_log.Scryfall.Bulk_data_updated_at.UTC().String())
	}
}