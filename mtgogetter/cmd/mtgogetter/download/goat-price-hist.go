package download

import (
	"fmt"
	"log"
	"path/filepath"

	"github.com/CramBL/mtgo-collection-manager/mtgogetter/pkg/mtgogetter"
	"github.com/spf13/cobra"
)

const GoatbotsPriceHistoryUrl string = "https://www.goatbots.com/download/price-history.zip"

var DownloadGoatbotsPriceHistoryCmd = &cobra.Command{
	Use:     "goatbots-price-history",
	Aliases: []string{"goat-price-hist", "goat-ph", "gph"},
	Short:   "Download the price history for cards on Goatbots.com",
	Long: `Download the price history for cards on Goatbots.com

The price history appears as a JSON map of unique card IDs and associated tix price`,
	Args: cobra.ExactArgs(0),
	RunE: func(cmd *cobra.Command, args []string) error {
		var working_dir string = "default"      // default is the current working directory
		var fname string = "price-history.json" // default filename
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

		// First check if the price history is up to date
		// If it is, then we don't need to download it again

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

		if state_log.Goatbots.IsPriceUpdated() {
			log.Println("Price history is up to date - no need to download")
			return nil
		}

		log.Println("Price history is out of date - downloading")

		// Update the timestamp in the state log after downloading the price history
		// Only runs if the download is successful (no call to log.Fatalln()/os.Exit())
		defer state_log.Goatbots.UpdatePriceTimestamp(working_dir)

		dl_bytes, err := mtgogetter.DownloadBodyToBytes(GoatbotsPriceHistoryUrl)
		if err != nil {
			return fmt.Errorf("error downloading price history: %s", err)
		}
		reader, err := mtgogetter.UnzipFromBytes(dl_bytes)
		if err != nil {
			return fmt.Errorf("error unzipping price history: %s", err)
		}

		first_file_from_zip, err := mtgogetter.FirstFileFromZip(reader)
		if err != nil {
			return fmt.Errorf("error getting first file from zip: %s", err)
		}

		if working_dir != "default" {
			_, err := mtgogetter.ReadCloserToPath(first_file_from_zip, fname)
			if err != nil {
				return fmt.Errorf("error writing file to disk: %s", err)
			}
		} else if mtgogetter.OutputIsStdout(cmd) {
			// If the --save-as flag was not set (or is set to stdout), print to stdout
			_, err := mtgogetter.ReadCloserToStdout(first_file_from_zip)
			if err != nil {
				return fmt.Errorf("error writing file to stdout: %s", err)
			}
		} else {
			fname := cmd.Flag("save-as").Value.String()
			_, err := mtgogetter.ReadCloserToDisk(first_file_from_zip, fname)
			if err != nil {
				return fmt.Errorf("error writing file to disk: %s", err)
			}
		}
		return nil
	},
}
