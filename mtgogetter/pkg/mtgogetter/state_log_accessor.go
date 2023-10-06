package mtgogetter

import (
	"errors"
	"log"
	"os"
	"path/filepath"
	"sync"

	"github.com/BurntSushi/toml"
)

type StateLogAccesor struct {
	mut       sync.Mutex
	Path      string
	state_log *StateLog
}

var stateLogAcessorInstance *StateLogAccesor = nil

var stateLogWithDefault string = "state_log.toml"

// Get the state log accessor singleton
func GetStateLogAccessor(log_dir string) (*StateLogAccesor, error) {
	var logPath string
	if log_dir != "default" {
		logPath = filepath.Join(log_dir, stateLogWithDefault)
	} else {
		logPath = stateLogWithDefault
	}

	state_log, err := getStateLog(logPath)
	if err != nil {
		return nil, err
	}

	if stateLogAcessorInstance == nil {
		stateLogAcessorInstance = &StateLogAccesor{
			mut:       sync.Mutex{},
			Path:      logPath,
			state_log: state_log,
		}
	}
	return stateLogAcessorInstance, nil
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
	err := writeStateLogToFile(s.state_log, s.Path)
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
	err := writeStateLogToFile(s.state_log, s.Path)
	if err != nil {
		return err
	}
	return nil
}

// Below are private helper functions for the state log accessor

// Get the state log from disk if it exists, otherwise create a new one
func getStateLog(path_to_log string) (*StateLog, error) {
	var stateLog *StateLog

	if stateLogExists(path_to_log) {
		if _, err := toml.DecodeFile(path_to_log, &stateLog); err != nil {
			return nil, err
		}
	} else {
		stateLog = NewStateLog()
		if err := writeStateLogToFile(stateLog, path_to_log); err != nil {
			return nil, err
		}
	}

	return stateLog, nil
}

func writeStateLogToFile(stateLog *StateLog, logPath string) error {
	dir := filepath.Dir(logPath)
	err := os.MkdirAll(dir, 0777)
	if err != nil {
		return err
	}

	f, err := os.Create(logPath)

	if err != nil {
		return err
	}
	defer f.Close()

	if err := toml.NewEncoder(f).Encode(stateLog); err != nil {
		return err
	}

	return nil
}

func stateLogExists(path_to_log string) bool {
	if _, err := os.Stat(path_to_log); err == nil {
		return true
	} else if errors.Is(err, os.ErrNotExist) {
		// Doesn't exist should be created
		return false
	} else {
		log.Println(err)
		return false
	}
}
