package custom

import (
	"log"
	"os"

	"github.com/CramBL/mtgo-collection-manager/mtgogetter/pkg/mtgogetter"
	"github.com/spf13/cobra"
)

var DownloadCustomUrlStringCmd = &cobra.Command{
	Use:     "url-raw",
	Aliases: []string{"url-body", "url-dump"},
	Short:   "Download from a custom (user defined) URL and save the response body to a file",
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
