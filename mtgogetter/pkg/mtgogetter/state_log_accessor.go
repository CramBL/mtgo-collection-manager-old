package mtgogetter

import (
	"errors"
	"log"
	"os"
	"sync"

	"github.com/BurntSushi/toml"
)

type StateLogAccesor struct {
	mut       sync.Mutex
	state_log *StateLog
}

var instance *StateLogAccesor = nil

const stateLogPath string = "state_log.toml"

// Get the state log accessor singleton
func GetStateLogAccessor() (*StateLogAccesor, error) {
	state_log, err := getStateLog()
	if err != nil {
		return nil, err
	}

	if instance == nil {
		instance = &StateLogAccesor{
			mut:       sync.Mutex{},
			state_log: state_log,
		}
	}
	return instance, nil
}

// Get the state log.
// REMEMBER TO CALL ReleaseStateLog() or another unlocking method when done (use defer if possible)
func (s *StateLogAccesor) GetStateLog() *StateLog {
	s.mut.Lock()
	return s.state_log
}

// Release the state log
func (s *StateLogAccesor) ReleaseStateLog() {
	s.mut.Unlock()
}

// Update the state log by passing a function that does the update
func (s *StateLogAccesor) UpdateStateLog(update_action func(*StateLog)) error {
	s.mut.Lock()
	defer s.mut.Unlock()

	update_action(s.state_log)

	// Write the state log to disk
	err := writeStateLogToFile(s.state_log)
	if err != nil {
		return err
	}
	return nil
}

// Update and unlock an already locked state log
// Runtime error if the state log is not locked
func (s *StateLogAccesor) UpdateAndUnlockStateLog(update_action func(*StateLog)) error {
	update_action(s.state_log)
	defer s.mut.Unlock()

	// Write the state log to disk
	err := writeStateLogToFile(s.state_log)
	if err != nil {
		return err
	}
	return nil
}

// Below are private helper functions for the state log accessor

// Get the state log from disk if it exists, otherwise create a new one
func getStateLog() (*StateLog, error) {
	var stateLog *StateLog
	if stateLogExists() {
		if _, err := toml.DecodeFile(stateLogPath, &stateLog); err != nil {
			return nil, err
		}
	} else {
		stateLog = NewStateLog()
		if err := writeStateLogToFile(stateLog); err != nil {
			return nil, err
		}
	}

	return stateLog, nil
}

func writeStateLogToFile(stateLog *StateLog) error {
	f, err := os.Create(stateLogPath)
	if err != nil {
		return err
	}
	defer f.Close()

	if err := toml.NewEncoder(f).Encode(stateLog); err != nil {
		return err
	}

	return nil
}

func stateLogExists() bool {
	if _, err := os.Stat(stateLogPath); err == nil {
		return true
	} else if errors.Is(err, os.ErrNotExist) {
		// Doesn't exist should be created
		return false
	} else {
		log.Println(err)
		return false
	}
}
