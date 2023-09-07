package custom

import (
	"io"
	"log"
	"os"
	"path/filepath"

	"github.com/CramBL/mtgo-collection-manager/mtgogetter/pkg/mtgogetter"
	"github.com/spf13/cobra"
)

var DownloadCustomUrlStringCmd = &cobra.Command{
	Use:     "url-raw",
	Aliases: []string{"url-body", "url-dump"},
	Short:   "Download from a custom (user defined) URL and write the response body to stdout or a specified file",
	Run: func(cmd *cobra.Command, args []string) {
		dl_url_arg := args[0]
		do_decompress, err := cmd.Flags().GetBool("decompress")
		if err != nil {
			log.Fatalln("Error retrieving decompress flag value from args")
		}
		if do_decompress {
			extension := filepath.Ext(dl_url_arg)
			// Only supports .zip as of now
			if extension != ".zip" {
				log.Fatalln("Decompression specified but URL does not specify compressed content")
			}
		}

		dl_bytes := mtgogetter.DownloadBodyToBytes(dl_url_arg)

		if mtgogetter.OutputIsStdout(cmd) {
			if do_decompress {
				reader := mtgogetter.UnzipFromBytes(dl_bytes)
				first_file_reader := reader.File[0]
				log.Println("Extracting:", first_file_reader.Name, "and printing to stdout")

				first_file, err := first_file_reader.Open()
				if err != nil {
					log.Fatalln("Error opening first file from zip archive: ", err)
				}

				// Print to stdout
				_, err = io.Copy(os.Stdout, first_file)
				if err != nil {
					log.Fatalln("Error printing to stdout:", err)
				}

			} else {
				os.Stdout.Write(dl_bytes)
			}
		} else {
			fname := cmd.Flag("save-as").Value.String()

			if do_decompress {
				reader := mtgogetter.UnzipFromBytes(dl_bytes)
				mtgogetter.FirstFileFromZipToDisk(fname, reader)
			} else {
				// Create file on disk for writing
				err = os.WriteFile(fname, dl_bytes, 0777)
				if err != nil {
					log.Println("Error writing file:", err)
				}
			}
		}
	},
}
