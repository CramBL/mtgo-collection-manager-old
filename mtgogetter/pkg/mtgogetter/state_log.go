package mtgogetter

import (
	"errors"
	"log"
	"os"
	"time"

	"github.com/BurntSushi/toml"
)

type goatbots struct {
    Card_definitions_updated_at  time.Time
    Prices_updated_at  time.Time
}

type scryfall struct {
    Updated_at  time.Time
}

type StateLog struct {
    Title string
    Goatbots    goatbots `toml:"goatbots"`
    Scryfall    scryfall `toml:"scryfall"`
}


const StateLogPath string = "state_log.toml"

func NewStateLog() *StateLog {
    return &StateLog{
        Title: "log for MTGO Getter state, such as updated_at timestamps",
        // Set time stamps to Unix epoch to signify that they have not been updated yet
        Goatbots: goatbots{
            Card_definitions_updated_at: time.Unix(0, 0),
            Prices_updated_at: time.Unix(0, 0),
        },
        Scryfall: scryfall{
            Updated_at: time.Unix(0, 0),
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