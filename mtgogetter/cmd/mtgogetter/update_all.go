package mtgogetter

import (
	"fmt"
	"log"
	"sync"

	"github.com/CramBL/mtgo-collection-manager/mtgogetter/cmd/mtgogetter/download"
	"github.com/spf13/cobra"
)

func do_work(work_func func() error, wg *sync.WaitGroup, err_chan chan<- error) {
	defer wg.Done()
	if err := work_func(); err != nil {
		err_chan <- err
	}
}

var UpdateAllCmd = &cobra.Command{
	Use:     "update",
	Aliases: []string{"update-all", "run-all-downloads", "update-all-downloads"},
	Short:   "Update all downloaded data",
	Args:    cobra.ExactArgs(0),
	PreRun: func(cmd *cobra.Command, args []string) {
		// Set the save-as flag for all subcommands
		if err := download.DownloadGoatbotsPriceHistoryCmd.Flag("save-as").Value.Set("price-history.json"); err != nil {
			log.Fatalf("error setting flag: %s", err)
		}
		download.DownloadGoatbotsPriceHistoryCmd.Flag("save-as").Changed = true
		if err := download.DownloadGoatbotsCardDefinitionsCmd.Flag("save-as").Value.Set("card-definitions.json"); err != nil {
			log.Fatalf("error setting flag: %s", err)
		}
		download.DownloadGoatbotsCardDefinitionsCmd.Flag("save-as").Changed = true
	},
	RunE: func(cmd *cobra.Command, args []string) error {
		target_dir := cmd.Flag("save-to-dir").Value.String()
		log.Println("Updating all downloaded data and saving updates to", target_dir)

		var go_routine_count int = 3
		var work_group sync.WaitGroup
		// Define a channel for error reporting
		// Buffer the channel by the goroutine count so that it doesn't block on reported errors
		error_channel := make(chan error, go_routine_count)

		log.Println("Updating goatbots price history")
		dl_gb_price_history := func() error {
			return download.DownloadGoatbotsPriceHistoryCmd.RunE(cmd, []string{"--save-to-dir", target_dir, "--save-as", "price-history.json"})
		}
		work_group.Add(1)
		go do_work(dl_gb_price_history, &work_group, error_channel)

		log.Println("Updating goatbots card definitions")
		dl_gb_card_definitions := func() error {
			var any_errs_str string = ""
			err := download.DownloadScryfallSetListCmd.RunE(cmd, []string{"--save-to-dir", target_dir, "--save-as", "scryfall-sets.json"})
			if err != nil {
				any_errs_str = err.Error()
			}
			err = download.DownloadGoatbotsCardDefinitionsCmd.RunE(cmd, []string{"--save-to-dir", target_dir, "--save-as", "card-definitions.json"})
			if err != nil {
				any_errs_str = any_errs_str + " " + err.Error()
			}
			if any_errs_str != "" {
				return fmt.Errorf("error(s) encountered during card definition updates: %s", any_errs_str)
			}

			return nil
		}
		work_group.Add(1)
		go do_work(dl_gb_card_definitions, &work_group, error_channel)

		log.Println("Updating scryfall bulk data")
		dl_scryfall_bulk := func() error {
			return download.DownloadScryfallBulkCmd.RunE(cmd, []string{"--save-to-dir", target_dir, "--save-as", "scryfall-bulk.json"})
		}
		work_group.Add(1)
		go do_work(dl_scryfall_bulk, &work_group, error_channel)

		log.Println("Waiting for all downloads to finish")
		work_group.Wait()

		select {
		// If there was any error the command failed
		case err := <-error_channel:
			log.Fatalln("Error updating downloaded data:", err)
			return err
		default:
			log.Println("All downloads finished successfully")
			return nil
		}
	},
}

func init() {
	RootCmd.AddCommand(UpdateAllCmd)
	var save_as_file_name string
	UpdateAllCmd.PersistentFlags().StringVarP(&save_as_file_name, "save-as", "s", "stdout", "Write downloaded content to specified filename")
	UpdateAllCmd.Flag("save-as").Hidden = true
	var save_to_dir string
	UpdateAllCmd.Flags().StringVarP(&save_to_dir, "save-to-dir", "d", "", "Write downloaded content to specified directory")
	download.DownloadGoatbotsPriceHistoryCmd.Hidden = true
	UpdateAllCmd.AddCommand(download.DownloadGoatbotsPriceHistoryCmd)
	download.DownloadGoatbotsCardDefinitionsCmd.Hidden = true
	UpdateAllCmd.AddCommand(download.DownloadGoatbotsCardDefinitionsCmd)
	download.DownloadScryfallBulkCmd.Hidden = true
	UpdateAllCmd.AddCommand(download.DownloadScryfallBulkCmd)
	download.DownloadScryfallSetListCmd.Hidden = true
}
