package mtgogetter_test

import (
	"log"
	"testing"

	. "github.com/CramBL/mtgo-collection-manager/mtgogetter/pkg/mtgogetter"
)

func TestDownloadBodyToBytes(t *testing.T) {
    got_body_bytes := DownloadBodyToBytes("https://www.github.com/CramBL/mtgo-collection-manager")
    log.Println("Got body len ==", len(got_body_bytes))
    if len(got_body_bytes) == 0 {
        t.Errorf("Expected response body from DownloadBodyToBytes, got is empty! ")
    }
}