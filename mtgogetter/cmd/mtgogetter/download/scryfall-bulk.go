package download

import (
	"encoding/json"
	"fmt"
	"io"
	"log"
	"net/http"
	"strings"
	"time"

	"github.com/CramBL/mtgo-collection-manager/mtgogetter/pkg/mtgogetter"
	"github.com/spf13/cobra"
)

// Structs for JSON unmarshalling the response from the Scryfall API when requesting bulk data metadata, such as download uri, updated_at, size, etc.
type ScryfallBulkDataInfo struct {
	Download_uri string `json:"download_uri"`
	Updated_at   string `json:"updated_at"`
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
	RunE: func(cmd *cobra.Command, args []string) error {
		// 1. get the bulk data info from the Scryfall API.
		// 	  It contains the download uri and updated_at timestamp
		// 2. Check if the bulk data has been updated since we last downloaded it.
		// 	  If it hasn't been updated, there's no need to download it again
		//(3.) IF new bulk data is available: Download the bulk data from the Scryfall API

		log.Println("GET bulk data info from:", ScryfallInfoBulkUrl)
		resp, err := http.Get(ScryfallInfoBulkUrl)
		if err != nil {
			return fmt.Errorf("error when getting bulk data info: %s", err)
		}
		defer resp.Body.Close()

		if resp.StatusCode != 200 {
			return fmt.Errorf("GET returned: %d %s", resp.StatusCode, http.StatusText(resp.StatusCode))
		}

		bodyAsBytes, err := io.ReadAll(resp.Body)
		if err != nil {
			return fmt.Errorf("error when reading response body: %s", err)
		}

		var msg ScryfallBulkDataInfo
		if err := json.Unmarshal(bodyAsBytes, &msg); err != nil {
			return fmt.Errorf("error when unmarshalling response body: %s", err)
		}
		log.Println("Response contained updated_at:", msg.Updated_at)
		log.Println("Response contained download uri:", msg.Download_uri)

		// Check if the bulk data has been updated since we last downloaded it.
		// If it hasn't been updated, there's no need to download it again
		state_log_accesor, err := mtgogetter.GetStateLogAccessor()
		if err != nil {
			return fmt.Errorf("error getting state log accessor: %s", err)
		}
		state_log := state_log_accesor.GetStateLog()
		// Release the state log immediately
		// Assumes that the state log will not be used again for the same purpose as in this command
		// while this command is running
		// The way this breaks is if this command is run in parallel with itself which is faulty use
		state_log_accesor.ReleaseStateLog()
		if err != nil {
			return fmt.Errorf("error getting state log: %s", err)
		}

		t_updated_at, err := time.Parse(time.RFC3339, msg.Updated_at)
		if err != nil {
			return fmt.Errorf("error parsing timestamp from response body: %s", err)
		}

		if state_log.Scryfall.IsBulkDataUpdated(t_updated_at) {
			log.Println("Bulk data is up to date - no need to download")
			return nil
		}

		// Update the timestamp in the state log after downloading the bulk data
		defer state_log.Scryfall.UpdateBulkDataTimestamp()

		var split_msg = strings.SplitAfter(msg.Download_uri, "default-cards-")
		// Concatenate the end (timestamp + .json) of the received download_uri with what we know the prefix URL should be
		// simply to not blindly call GET on unsanitized received URL
		download_url := templateDownloadUri + split_msg[1]
		log.Println("Downloading bulk data from:", download_url)

		resp_bulk_data, err := http.Get(download_url)
		if err != nil {
			return fmt.Errorf("error when getting bulk data: %s", err)
		}
		defer resp.Body.Close()
		if resp_bulk_data.StatusCode != 200 {
			return fmt.Errorf("GET bulk data returned: %d %s", resp_bulk_data.StatusCode, http.StatusText(resp_bulk_data.StatusCode))
		}

		// Name is default-cards-<timestamp>.json
		fname := "default-cards-" + split_msg[1]

		log.Println("Deserializing raw scryfall JSON from stream")
		stream_decoder := json.NewDecoder(resp_bulk_data.Body)

		// Deserialize to the ScryfallCard struct (Taking only the fields we need)
		scryfall_cards, err := mtgogetter.ScryfallCardsFromJsonStream(stream_decoder)
		if err != nil {
			return fmt.Errorf("error when deserializing JSON stream: %s", err)
		}

		log.Println("Serializing scryfall card array to JSON and writing to disk as:", fname)
		// Serialize to JSON string and write to disk
		if err := mtgogetter.ScryfallCardsToDisk(scryfall_cards, fname); err != nil {
			return fmt.Errorf("error when serializing scryfall cards to disk: %s", err)
		}
		return nil
	},
}
