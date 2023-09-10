package download

import (
	"fmt"

	"github.com/CramBL/mtgo-collection-manager/mtgogetter/pkg/mtgogetter"
	"github.com/spf13/cobra"
)

const GoatbotsCardDefinitionsUrl string = "https://www.goatbots.com/download/card-definitions.zip"

var DownloadGoatbotsCardDefinitionsCmd = &cobra.Command{
	Use:     "goatbots-card-definitions",
	Aliases: []string{"goat-card-defs", "goat-cd", "gcd"},
	Short:   "Download card information (definitions) on Goatbots.com",
	Long: `Download card information (definitions) on Goatbots.com.

Card definitions includes a unique card ID with associated name, cardset, rarity, and foil (0/1)`,
	Args: cobra.ExactArgs(0),
	RunE: func(cmd *cobra.Command, args []string) error {

		dl_bytes, err := mtgogetter.DownloadBodyToBytes(GoatbotsCardDefinitionsUrl)
		if err != nil {
			return fmt.Errorf("error downloading card definitions: %s", err)
		}
		reader, err := mtgogetter.UnzipFromBytes(dl_bytes)
		if err != nil {
			return fmt.Errorf("error unzipping card definitions: %s", err)
		}

		first_file_from_zip, err := mtgogetter.FirstFileFromZip(reader)
		if err != nil {
			return fmt.Errorf("error getting first file from zip: %s", err)
		}

		// If the --save-as flag was not set (or is set to stdout), print to stdout
		if mtgogetter.OutputIsStdout(cmd) {
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
