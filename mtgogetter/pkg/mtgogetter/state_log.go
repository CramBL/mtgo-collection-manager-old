// This file contains the structs and methods for the state log
// To access the state log use the state_log_accessor.go file
// it contains a singleton that handles thread safe access to the state log
// If you want to update or read specific fields in the state log
// you can use the methods in this file

package mtgogetter

import (
	"log"
	"time"
)

type goatbots struct {
	Card_definitions_updated_at time.Time
	Prices_updated_at           time.Time
}

// Method for the goatbots struct to check if the price data is up to date.
// it's outdated if it hasn't been updated since 4 AM UTC
func (g *goatbots) IsPriceUpdated() bool {
	// Get the current time
	utc_now := time.Now().UTC()
	utc_4am := time.Date(utc_now.Year(), utc_now.Month(), utc_now.Day(), 4, 0, 0, 0, time.UTC)

	// If the current time is before 4 AM, then we want to check if the prices were updated yesterday
	if utc_now.Before(utc_4am) {
		utc_4am_yesterday := time.Date(utc_now.Year(), utc_now.Month(), utc_now.Day()-1, 4, 0, 0, 0, time.UTC)
		return g.Prices_updated_at.After(utc_4am_yesterday)
	} else {
		// If the current time is after 4 AM, then we want to check if the prices were updated today
		return g.Prices_updated_at.After(utc_4am)
	}
}

// Method for the goatbots struct to generate a new timestamp for the price data
// This should be called after the price data is downloaded
// It will then load the state log from disk and update the timestamp
func (g *goatbots) UpdatePriceTimestamp(stateLogPath string) error {
	state_log_accesor, err := GetStateLogAccessor(stateLogPath)
	if err != nil {
		return err
	}
	update_action := func(state_log *StateLog) {
		state_log.Goatbots.Prices_updated_at = time.Unix(time.Now().UTC().Unix(), 0).UTC()
	}
	err = state_log_accesor.UpdateStateLog(update_action)
	if err != nil {
		return err
	}
	return nil
}

// Method for the goatbots struct to check if the card definitions are up to date.
// it's outdated if a new set has been released since the last update
func (g *goatbots) IsCardDefinitionsUpdated() bool {
	log.Fatalln("Not implemented yet")
	return false
}

// Method for the goatbots struct to generate a new timestamp for the card definitions
func (g *goatbots) UpdateCardDefinitionsTimestamp(stateLogPath string) error {
	state_log_accesor, err := GetStateLogAccessor(stateLogPath)
	if err != nil {
		return err
	}

	update_action := func(state_log *StateLog) {
		state_log.Goatbots.Card_definitions_updated_at = time.Unix(time.Now().UTC().Unix(), 0).UTC()
	}
	err = state_log_accesor.UpdateStateLog(update_action)
	if err != nil {
		return err
	}

	return nil
}

type scryfall struct {
	// Bulk data is updated every 12 hours
	Bulk_data_updated_at time.Time
}

// Method for the scryfall struct to check if the bulk data is up to date.
// outdated if the timestamp is older than the `updated_at` retrieved from the Scryfall API
func (s *scryfall) IsBulkDataUpdated(api_timestamp time.Time) bool {
	return s.Bulk_data_updated_at.After(api_timestamp)
}

// Method for the scryfall struct to generate a new timestamp for the price data
// This should be called after the bulk data is downloaded
// It will then load the state log from disk and update the timestamp
func (s *scryfall) UpdateBulkDataTimestamp(stateLogPath string) error {

	state_log_accesor, err := GetStateLogAccessor(stateLogPath)
	if err != nil {
		return err
	}

	update_action := func(state_log *StateLog) {
		state_log.Scryfall.Bulk_data_updated_at = time.Unix(time.Now().UTC().Unix(), 0).UTC()
	}
	err = state_log_accesor.UpdateStateLog(update_action)
	if err != nil {
		return err
	}

	return nil
}

type StateLog struct {
	Title    string
	Goatbots goatbots `toml:"goatbots"`
	Scryfall scryfall `toml:"scryfall"`
}

func NewStateLog() *StateLog {
	return &StateLog{
		Title: "log for MTGO Getter state, such as updated_at timestamps",
		// Set time stamps to Unix epoch to signify that they have not been updated yet
		Goatbots: goatbots{
			Card_definitions_updated_at: time.Unix(0, 0).UTC(),
			Prices_updated_at:           time.Unix(0, 0).UTC(),
		},
		Scryfall: scryfall{
			Bulk_data_updated_at: time.Unix(0, 0).UTC(),
		},
	}
}
