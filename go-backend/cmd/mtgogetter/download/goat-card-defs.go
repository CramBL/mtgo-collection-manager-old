package download

import (
	"github.com/CramBL/mtgo-collection-manager/go-backend/pkg/mtgogetter"
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
		dl_bytes := mtgogetter.DownloadBodyToBytes(GoatbotsCardDefinitionsUrl)
		reader := mtgogetter.UnzipFromBytes(dl_bytes)
		mtgogetter.FirstFileFromZipToDisk("card-definitions.json", reader)
	},
}
