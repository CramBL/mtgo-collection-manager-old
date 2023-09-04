package mtgogetter

import (
	"github.com/CramBL/mtgo-collection-manager/mtgogetter/cmd/mtgogetter/download"
	"github.com/spf13/cobra"
)

var BaseDownloadCmd = &cobra.Command{
	Use:       "download",
	Aliases:   []string{"down", "dl"},
	Short:     "Download card information through a subcommand such as price history",
	ValidArgs: []string{"goatbots-price-history", "goatbots-card-definitions", "custom"},
	Args:      cobra.ExactArgs(0),
}

func init() {
	RootCmd.AddCommand(BaseDownloadCmd)

	BaseDownloadCmd.AddCommand(download.DownloadGoatbotsPriceHistoryCmd)
	BaseDownloadCmd.AddCommand(download.DownloadGoatbotsCardDefinitionsCmd)
	BaseDownloadCmd.AddCommand(download.DownloadScryfallBulkCmd)
	BaseDownloadCmd.AddCommand(download.BaseDownloadCustomCmd)
}
