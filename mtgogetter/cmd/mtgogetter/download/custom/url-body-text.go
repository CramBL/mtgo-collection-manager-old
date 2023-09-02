package custom

import (
	"log"
	"os"

	"github.com/CramBL/mtgo-collection-manager/mtgogetter/pkg/mtgogetter"
	"github.com/spf13/cobra"
)

var DownloadCustomUrlStringCmd = &cobra.Command{
	Use:     "url-string",
	Aliases: []string{"url-text", "url-body-text"},
	Short:   "Download the from a custom (user defined) URL",
	Run: func(cmd *cobra.Command, args []string) {
		fname := cmd.Flag("save-as").Value.String()
		dl_bytes := mtgogetter.DownloadBodyToBytes(args[0])
		// Create file on disk for writing
		err := os.WriteFile(fname, dl_bytes, 0777)
		if err != nil {
			log.Println("Error writing file:", err)
		}
	},
}
