package download

import (
	"github.com/CramBL/mtgo-collection-manager/mtgogetter/cmd/mtgogetter/download/custom"
	"github.com/spf13/cobra"
)

var BaseDownloadCustomCmd = &cobra.Command{
	Use:     "custom",
	Aliases: []string{"c", "url", "endpoint"},
	Short:   "Download content from a specified URL and save it to a file",
	Args:    cobra.ExactArgs(0),
}

func init() {
	var do_decompress bool
	BaseDownloadCustomCmd.PersistentFlags().BoolVarP(&do_decompress, "decompress", "d", false, "Specify that the downloaded content should be decompressed before writing to disk")
	BaseDownloadCustomCmd.AddCommand(custom.DownloadCustomUrlStringCmd)
}
