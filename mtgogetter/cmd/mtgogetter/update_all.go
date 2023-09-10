package mtgogetter

import (
	"github.com/CramBL/mtgo-collection-manager/mtgogetter/cmd/mtgogetter/download"
	"github.com/spf13/cobra"
)

var UpdateAllCmd = &cobra.Command{
	Use:       "update",
	Aliases:   []string{"update-all", "run-all-downloads", "update-all-downloads"},
	Short:     "Update all downloaded data",
	Args:      cobra.ExactArgs(0),
	Run: func(cmd *cobra.Command, args []string) {
		cmd.SetArgs([]string{"goatbots-price-history"})
		go download.DownloadGoatbotsPriceHistoryCmd.Run(cmd, args)
		cmd.SetArgs([]string{"goatbots-card-definitions"})
		go download.DownloadGoatbotsCardDefinitionsCmd.Run(cmd, args)
		cmd.SetArgs([]string{"scryfall-bulk-data"})
		go download.DownloadScryfallBulkCmd.Run(cmd, args)
	},
}

func init() {
	RootCmd.AddCommand(UpdateAllCmd)
}