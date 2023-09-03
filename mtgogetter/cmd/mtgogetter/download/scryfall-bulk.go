package download

import (
	"encoding/json"
	"io"
	"log"
	"net/http"

	"github.com/spf13/cobra"
)

const ScryfallInfoBulkUrl string = "https://api.scryfall.com/bulk-data/e2ef41e3-5778-4bc2-af3f-78eca4dd9c23"

var DownloadScryfallBulkCmd = &cobra.Command{
	Use:     "scryfall-bulk-data",
	Aliases: []string{"scryfall-bulk", "scryfall-bd", "sbd"},
	Short:   "Download bulk card data from the Scryfall API",
	Long: `Download bulk card data from the Scryfall API.
The data comes as a JSON file containing every card object on Scryfall in English or the printed language if the card is only available in one language.`,
	Args: cobra.ExactArgs(0),
	Run: func(cmd *cobra.Command, args []string) {
		log.Println("Downloading from", ScryfallInfoBulkUrl)
		resp, err := http.Get(ScryfallInfoBulkUrl)
		if err != nil {
			log.Fatal(err)
		}
		defer resp.Body.Close()
		if resp.StatusCode != 200 {
			log.Fatalln("Get returned:", resp.StatusCode, http.StatusText(resp.StatusCode))
		}

		bodyAsBytes, err := io.ReadAll(resp.Body)
		if err != nil {
			log.Fatalln(err)
		}

		var data map[string]interface{}
		err = json.Unmarshal(bodyAsBytes, &data)
		if err != nil {
			log.Fatalln("Error when Unmarshalling JSON:", err)
		}
		download_uri := data["download_uri"]
		log.Println("Got download uri:", download_uri)

	},
}
