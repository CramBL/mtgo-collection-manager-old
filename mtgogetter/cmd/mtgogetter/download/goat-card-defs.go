package download

import (
	"log"

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
	Run: func(cmd *cobra.Command, args []string) {
		dl_bytes, err := mtgogetter.DownloadBodyToBytes(GoatbotsCardDefinitionsUrl)
		if err != nil {
			log.Fatalln("Error downloading body:", err)
		}
		reader, err := mtgogetter.UnzipFromBytes(dl_bytes)
		if err != nil {
			log.Fatalln("Error unzipping bytes:", err)
		}

		first_file_from_zip, err := mtgogetter.FirstFileFromZip(reader)
		if err != nil {
			log.Fatalln("Error opening first file from zip archive: ", err)
		}

		// If the --save-as flag was not set (or is set to stdout), print to stdout
		if mtgogetter.OutputIsStdout(cmd) {
			_, err := mtgogetter.ReadCloserToStdout(first_file_from_zip)
			if err != nil {
				log.Fatalln("Error printing to stdout:", err)
			}
		} else {
			fname := cmd.Flag("save-as").Value.String()
			_, err := mtgogetter.ReadCloserToDisk(first_file_from_zip, fname)
			if err != nil {
				log.Fatalln("Error writing file:", err)
			}
		}
	},
}
