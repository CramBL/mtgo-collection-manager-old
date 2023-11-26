package download

import (
	"fmt"
	"log"
	"os"
	"path/filepath"
	"time"

	"github.com/CramBL/mtgo-collection-manager/mtgogetter/pkg/mtgogetter"
	"github.com/spf13/cobra"
)

const ScryfallSetListUrl string = "https://api.scryfall.com/sets"

var DownloadScryfallSetListCmd = &cobra.Command{

	Use:     "scryfall-sets",
	Aliases: []string{"scryfall-sets-list", "scry-sets"},
	Short:   "Download a list of MTG sets from the Scryfall API and save it as sets.json",
	Args:    cobra.ExactArgs(0),
	RunE: func(cmd *cobra.Command, args []string) error {
		var working_dir string = "default"      // default is the current working directory
		var fname string = "scryfall-sets.json" // default filename
		// If we're being called from update all, we need to check the args
		if len(args) > 1 && args[0] == "--save-to-dir" {
			working_dir = args[1]
			// If args contains a filename
			if len(args) > 3 && args[2] == "--save-as" {
				fname = filepath.Join(working_dir, args[3])
			} else {
				fname = filepath.Join(working_dir, fname) // default filename
			}
		}

		// Get the state log accessor
		state_log_accesor, err := mtgogetter.GetStateLogAccessor(working_dir)
		if err != nil {
			return fmt.Errorf("error getting state log accessor: %s", err)
		}

		state_log := state_log_accesor.GetStateLog()
		// Release the state log immediately
		// Assumes that the state log will not be used again for the same purpose as in this command
		// while this command is running
		// The way this breaks is if this command is run in parallel with itself which is faulty use
		state_log_accesor.ReleaseStateLog()

		if !state_log.Scryfall.IsRecentSetOut() {
			// If the most recent set it not yet out, no reason to download new set data
			log.Println("Set data is up to date - no need to download")
			return nil
		}

		log.Println("Set data is out of date - downloading new set data")

		dl_bytes, err := mtgogetter.DownloadBodyToBytes(ScryfallSetListUrl)
		if err != nil {
			return fmt.Errorf("error downloading scryfall sets list: %s", err)
		}

		nextReleasedSet, err := mtgogetter.NextReleasedScryfallSetFromJsonBytes(dl_bytes, time.Now())
		if err != nil {
			return fmt.Errorf("failed decoding scryfall set json: %s", err)
		}

		state_log.Scryfall.UpdateNextSet(nextReleasedSet, working_dir)

		if working_dir != "default" {
			dir := filepath.Dir(fname)
			err := os.MkdirAll(dir, 0777)
			if err != nil {
				return err
			}
			fd, err := os.Create(fname)
			if err != nil {
				return err
			}
			defer fd.Close()
			os.WriteFile(fname, dl_bytes, 0777)
			if err != nil {
				return fmt.Errorf("error writing file to disk: %s", err)
			}

		} else if mtgogetter.OutputIsStdout(cmd) {
			// If the --save-as flag was not set (or is set to stdout), print to stdout
			fmt.Print(string(dl_bytes))
			if err != nil {
				return fmt.Errorf("error writing file to stdout: %s", err)
			}
		} else {
			fname := cmd.Flag("save-as").Value.String()
			os.WriteFile(fname, dl_bytes, 0777)
			if err != nil {
				return fmt.Errorf("error writing file to disk: %s", err)
			}
		}
		return nil

	},
}
