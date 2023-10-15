package download

import (
	"fmt"
	"os"

	"github.com/CramBL/mtgo-collection-manager/mtgogetter/pkg/mtgogetter"
	"github.com/spf13/cobra"
)

const ScryfallSetListUrl string = "https://api.scryfall.com/sets"

var DownloadScryfallSetListCmd = &cobra.Command{

	Use:     "scryfall-sets",
	Aliases: []string{"scryfall-sets-list", "scry-sets"},
	Short:   "Download a list of MTG sets from the Scryfall API and sae it as sets.json",
	Args:    cobra.ExactArgs(0),
	RunE: func(cmd *cobra.Command, args []string) error {

		dl_bytes, err := mtgogetter.DownloadBodyToBytes(ScryfallSetListUrl)
		if err != nil {
			return fmt.Errorf("error downloading scryfall sets list: %s", err)
		}

		if mtgogetter.OutputIsStdout(cmd) {
			// If the --save-as flag was not set (or is set to stdout), print to stdout
			fmt.Print(string(dl_bytes))
			if err != nil {
				return fmt.Errorf("error writing file to stdout: %s", err)
			}
		} else {
			fname := cmd.Flag("save-as").Value.String()
			os.WriteFile(fname, dl_bytes, 0777)
			if err != nil {
				return fmt.Errorf("error writing file to disk: %s", err)
			}
		}
		return nil

	},
}
