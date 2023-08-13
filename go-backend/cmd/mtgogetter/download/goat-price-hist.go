package download

import (
	"github.com/CramBL/mtgo-collection-manager/go-backend/pkg/mtgogetter"
	"github.com/spf13/cobra"
)

const GoatbotsPriceHistoryUrl string = "https://www.goatbots.com/download/price-history.zip"

var DownloadGoatbotsPriceHistoryCmd = &cobra.Command{
	Use:     "goatbots-price-history",
	Aliases: []string{"goat-price-hist", "goat-ph", "gph"},
	Short:   "Download the price history for cards on Goatbots.com",
	Args:    cobra.ExactArgs(0),
	Run: func(cmd *cobra.Command, args []string) {
		dl_bytes := mtgogetter.DownloadBodyToBytes(GoatbotsPriceHistoryUrl)
		reader := mtgogetter.UnzipFromBytes(dl_bytes)
		mtgogetter.FirstFileFromZipToDisk("price-history.json", reader)
	},
}
