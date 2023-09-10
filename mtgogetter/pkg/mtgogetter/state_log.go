package mtgogetter

import (
	"errors"
	"log"
	"os"
	"time"

	"github.com/BurntSushi/toml"
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
func (g *goatbots) UpdatePriceTimestamp() error {
	g.Prices_updated_at = time.Unix(time.Now().UTC().Unix(), 0).UTC()
	state_log, err := GetStateLog()
	if err != nil {
		return err
	}
	state_log.Goatbots.Prices_updated_at = g.Prices_updated_at
	if err := WriteStateLogToFile(state_log); err != nil {
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
func (g *goatbots) UpdateCardDefinitionsTimestamp() {
	g.Card_definitions_updated_at = time.Unix(time.Now().UTC().Unix(), 0).UTC()
}

type scryfall struct {
	Bulk_data_updated_at time.Time
}

type StateLog struct {
	Title    string
	Goatbots goatbots `toml:"goatbots"`
	Scryfall scryfall `toml:"scryfall"`
}

const StateLogPath string = "state_log.toml"

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

func WriteStateLogToFile(stateLog *StateLog) error {
	f, err := os.Create(StateLogPath)
	if err != nil {
		return err
	}
	defer f.Close()

	if err := toml.NewEncoder(f).Encode(stateLog); err != nil {
		return err
	}

	return nil
}

func GetStateLog() (*StateLog, error) {
	var stateLog *StateLog
	if StateLogExists() {
		if _, err := toml.DecodeFile(StateLogPath, &stateLog); err != nil {
			return nil, err
		}
	} else {
		stateLog = NewStateLog()
		if err := WriteStateLogToFile(stateLog); err != nil {
			return nil, err
		}
	}

	return stateLog, nil
}

func StateLogExists() bool {
	if _, err := os.Stat(StateLogPath); err == nil {
		return true
	} else if errors.Is(err, os.ErrNotExist) {
		// Doesn't exist should be created
		return false
	} else {
		log.Println(err)
		return false
	}
}
