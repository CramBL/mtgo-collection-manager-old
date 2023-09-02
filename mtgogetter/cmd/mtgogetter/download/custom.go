package download

import (
	"github.com/CramBL/mtgo-collection-manager/mtgogetter/cmd/mtgogetter/download/custom"
	"github.com/spf13/cobra"
)

var BaseDownloadCustomCmd = &cobra.Command{
	Use:     "custom",
	Aliases: []string{"c", "url", "endpoint"},
	Short:   "Download from a custom (user defined) URL in a specified format",
	Args: cobra.ExactArgs(0),
}



func init() {
	var Save_as_file string
	BaseDownloadCustomCmd.PersistentFlags().StringVarP(&Save_as_file, "save-as", "s", "mtgogetter_tmp.txt", "Write downloaded content to filename")
	BaseDownloadCustomCmd.AddCommand(custom.DownloadCustomUrlStringCmd)
}
