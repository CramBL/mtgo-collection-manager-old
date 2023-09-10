package download

import (
	"log"

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
	Run: func(cmd *cobra.Command, args []string) {
		// First check if the price history is up to date
		// If it is, then we don't need to download it again

		state_log, err := mtgogetter.GetStateLog()
		if err != nil {
			log.Fatalln("Error getting state log:", err)
		}

		if state_log.Goatbots.IsPriceUpdated() {
			log.Println("Price history is up to date - no need to download")
			return
		} else {
			log.Println("Price history is out of date - downloading")
		}

		// Update the timestamp in the state log after downloading the price history
		// Only runs if the download is successful (no call to log.Fatalln()/os.Exit())
		defer state_log.Goatbots.UpdatePriceTimestamp()

		dl_bytes, err := mtgogetter.DownloadBodyToBytes(GoatbotsPriceHistoryUrl)
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
