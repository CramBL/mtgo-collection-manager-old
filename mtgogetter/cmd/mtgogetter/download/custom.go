package download

import (
	"github.com/CramBL/mtgo-collection-manager/mtgogetter/cmd/mtgogetter/download/custom"
	"github.com/spf13/cobra"
)

var BaseDownloadCustomCmd = &cobra.Command{
	Use:     "custom",
	Aliases: []string{"c", "url", "endpoint"},
	Short:   "Download content from a specified URL and save it to a file",
	Args: cobra.ExactArgs(0),
}



func init() {
	var Save_as_file string
	BaseDownloadCustomCmd.PersistentFlags().StringVarP(&Save_as_file, "save-as", "s", "mtgogetter_tmp.txt", "Write downloaded content to specified filename")
	BaseDownloadCustomCmd.AddCommand(custom.DownloadCustomUrlStringCmd)
}
