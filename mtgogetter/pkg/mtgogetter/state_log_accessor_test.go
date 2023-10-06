package mtgogetter_test

import (
	"fmt"
	"os"
	"sync"
	"testing"
	"time"

	. "github.com/CramBL/mtgo-collection-manager/mtgogetter/pkg/mtgogetter"
)

func TestConcurrentAccess(t *testing.T) {
	// Tests two goroutines attempting to access the state log at the same time

	// Goroutine1:
	// 			  1. Get and Lock the state log
	//			  2. Sleep 100ms, then update the state log and unlock it
	//			  3. Sleep 100ms and then exit
	//
	// Goroutine2:
	// 			  1. Sleep 80ms to make sure the first goroutine locks the file
	//			  2. Attempt to get and lock the state log (will block waiting for goroutine1)
	//			  3. Once goroutine1 unlocks the state log, goroutine2 will get and lock it
	//			  4. Goroutine2 will check the state log, and if it hasn't been updated as expected it will error
	//			  5. Goroutine2 will update the state log and unlock it
	//
	// Finally, the test will check that the state log was updated as expected

	// Get the state log accessor
	state_log_accessor, err := GetStateLogAccessor("default")
	if err != nil {
		t.Fatal(err)
	}

	// Set time to 12th of December 2021
	first_time_update := time.Date(2021, 12, 12, 0, 0, 0, 0, time.UTC)
	// Second and final time set it to 12th of December 2022
	second_time_update := time.Date(2022, 12, 12, 0, 0, 0, 0, time.UTC)

	var work_group sync.WaitGroup

	// Define the goroutines

	goroutine1 := func(wg *sync.WaitGroup) {
		defer wg.Done()
		state_log := state_log_accessor.GetStateLog()
		fmt.Println("Goroutine 1: Got state log")
		fmt.Println("Card definitions updated at:", state_log.Goatbots.Card_definitions_updated_at)
		fmt.Println("Goroutine 1: Sleeping for 100ms")
		time.Sleep(100 * time.Millisecond)
		fmt.Println("Goroutine 1: Updating state log")
		// Update the state log
		state_log_accessor.UpdateAndUnlockStateLog(func(state_log *StateLog) {
			state_log.Goatbots.Card_definitions_updated_at = first_time_update
		})
		fmt.Println("Goroutine 1: Done updating state log")
		fmt.Println("Goroutine 1: Sleeping for 100ms and then exiting...")
		time.Sleep(100 * time.Millisecond)
	}

	goroutine2 := func(wg *sync.WaitGroup) {
		defer wg.Done()
		fmt.Println("Goroutine 2: Sleeping for 80ms")
		time.Sleep(80 * time.Millisecond)
		fmt.Println("Goroutine 2: Attempting to get state log")
		state_log := state_log_accessor.GetStateLog()
		fmt.Println("Goroutine 2: Got state log")
		fmt.Println("Card definitions updated at:", state_log.Goatbots.Card_definitions_updated_at)
		if state_log.Goatbots.Card_definitions_updated_at != first_time_update {
			t.Error("Goroutine 2: Card definitions updated at time is not equal to first time update")
			state_log_accessor.ReleaseStateLog()
		} else {
			// Update the state log
			state_log_accessor.UpdateAndUnlockStateLog(func(state_log *StateLog) {
				state_log.Goatbots.Card_definitions_updated_at = second_time_update
			})
		}
		fmt.Println("Goroutine 2: Done updating state log, exiting...")
	}

	// Run the goroutines
	work_group.Add(2)
	go goroutine1(&work_group)
	go goroutine2(&work_group)
	work_group.Wait()

	// Check that the state log was updated
	state_log := state_log_accessor.GetStateLog()
	defer state_log_accessor.ReleaseStateLog()
	if state_log.Goatbots.Card_definitions_updated_at != second_time_update {
		t.Error("Card definitions updated at time is not equal to second time update")
	}

	// Clean up by deleting the state log (it's in the same directory as this test file)
	os.Remove("state_log.toml")
}
