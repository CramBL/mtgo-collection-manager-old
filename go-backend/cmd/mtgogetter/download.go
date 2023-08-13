package mtgogetter

import (
	"github.com/CramBL/mtgo-collection-manager/go-backend/cmd/mtgogetter/download"
	"github.com/spf13/cobra"
)

const GoatbotsPriceHistoryUrl string = "https://www.goatbots.com/download/price-history.zip"
const GoatbotsCardDefinitionsUrl string = "https://www.goatbots.com/download/card-definitions.zip"

var BaseDownloadCmd = &cobra.Command{
	Use:       "download",
	Aliases:   []string{"down", "dl"},
	Short:     "Download card information through a subcommand such as price history",
	ValidArgs: []string{"goatbots-price-history", "goatbots-card-definitions", "something else"},
	Args:      cobra.ExactArgs(0),
}

func init() {
	RootCmd.AddCommand(BaseDownloadCmd)

	BaseDownloadCmd.AddCommand(download.DownloadGoatbotsPriceHistoryCmd)
	BaseDownloadCmd.AddCommand(download.DownloadGoatbotsCardDefinitionsCmd)
}
