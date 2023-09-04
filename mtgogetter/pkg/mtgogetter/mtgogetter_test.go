package mtgogetter_test

import (
	"log"
	"testing"

	. "github.com/CramBL/mtgo-collection-manager/mtgogetter/pkg/mtgogetter"
)

// TODO: Mock HTTP requests: https://www.thegreatcodeadventure.com/mocking-http-requests-in-golang/
func TestDownloadBodyToBytes(t *testing.T) {
    got_body_bytes := DownloadBodyToBytes("https://raw.githubusercontent.com/CramBL/mtgo-collection-manager/master/LICENSE")
    log.Println("Got body len ==", len(got_body_bytes))
    if len(got_body_bytes) != 1072 {
        t.Errorf("Expected response body size of 1072 from DownloadBodyToBytes, got %d", len(got_body_bytes))
    }

    body_as_string := string(got_body_bytes)

    if body_as_string[0:11] != "MIT License" {
        t.Errorf("Expected body to start with 'MIT License', got %s", body_as_string[0:11])
    }
}