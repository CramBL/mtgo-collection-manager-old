package main

import (
	"log"
	"os"

	"github.com/CramBL/mtgo-collection-manager/mtgogetter/cmd/mtgogetter"
)

func main() {
	log.SetOutput(os.Stderr)
	mtgogetter.Execute()
}
