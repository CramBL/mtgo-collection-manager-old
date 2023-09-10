package mtgogetter

import (
	"sync"
)

type StateLogAccesor struct {
	mut 	  sync.Mutex
	state_log *StateLog
}

var instance *StateLogAccesor = nil

// Get the state log accessor singleton
func GetStateLogAccessor() (*StateLogAccesor, error) {
	state_log, err := GetStateLog()
	if err != nil {
		return nil, err
	}

	if instance == nil {
		instance = &StateLogAccesor{
			mut: sync.Mutex{},
			state_log: state_log,
		}
	}
	return instance, nil
}

// Get the state log.
// REMEMBER TO CALL ReleaseStateLog() WHEN DONE (use defer)
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
	err := writeStateLogToFile(s.state_log); if err != nil {
		return err
	}
	return nil
}
