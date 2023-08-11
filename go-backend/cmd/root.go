package mtgogetter

import (
	"log"

	"github.com/spf13/cobra"
)

var version = "0.1.0"
var rootCmd = &cobra.Command{
	Use:     "mtgogetter",
	Version: version,
	Short:   "MTGO Getter - a simple utility to get/download MTGO card info",
	Long:    "Includes commands to download and unzip files",
	Run: func(cmd *cobra.Command, args []string) {

	},
}

func Execute() {
	if err := rootCmd.Execute(); err != nil {
		log.Fatalf("Init error: '%s'", err)
	}
}
