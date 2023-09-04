package download

import (
	"encoding/json"
	"io"
	"log"
	"net/http"
	"strings"

	"github.com/CramBL/mtgo-collection-manager/mtgogetter/pkg/mtgogetter"
	"github.com/spf13/cobra"
)

type Message struct {
	Download_uri string `json:"download_uri"`
}

const ScryfallInfoBulkUrl string = "https://api.scryfall.com/bulk-data/e2ef41e3-5778-4bc2-af3f-78eca4dd9c23"

// Example download uri: https://data.scryfall.io/default-cards/default-cards-20230902211313.json
const templateDownloadUri string = "https://data.scryfall.io/default-cards/default-cards-" // Needs timestamp + ".json"

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

		var msg Message
		if err:= json.Unmarshal(bodyAsBytes, &msg); err != nil {
			log.Fatalln("Error when Unmarshalling JSON:", err)
		}
		log.Println("Got download uri:", msg.Download_uri)

		var split_msg = strings.SplitAfter(msg.Download_uri, "default-cards-")
		// Concatenate the end (timestamp + .json) of the received download_uri with what we know the prefix URL should be
		// simply to not blindly call GET on unsanitized received URL
		download_url := templateDownloadUri + split_msg[1]
		log.Println("Download URL to get bulk data from:", download_url)

		resp_bulk_data, err := http.Get(download_url)
		if err != nil {
			log.Fatal(err)
		}
		if resp_bulk_data.StatusCode != 200 {
			log.Fatalln("Get returned:", resp_bulk_data.StatusCode, http.StatusText(resp_bulk_data.StatusCode))
		}
		defer resp.Body.Close()

		// Name is default-cards-<timestamp>.json
		fname := "default-cards-" + split_msg[1]

		// Write resp_bulk_data.Body to file
		bulk_data, err :=io.ReadAll(resp_bulk_data.Body)
		if err != nil {
			log.Println("Error reading bulk data response body:", err)
		}

		// Deserialize to the ScryfallCard struct (Taking only the fields we need)
		scryfall_cards, err := mtgogetter.DeserializeScryfallCards(bulk_data)
		if err != nil {
			log.Fatalln("Error when deserializing Scryfall JSON:", err)
		}
		// Serialize to JSON string and write to disk
		if err := mtgogetter.ScryfallCardsToDisk(scryfall_cards, fname); err != nil {
			log.Fatalln("Error when writing Scryfall JSON to disk:", err)
		}
	},
}
