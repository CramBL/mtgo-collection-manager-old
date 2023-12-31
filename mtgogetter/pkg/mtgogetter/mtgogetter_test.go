package mtgogetter_test

import (
	"log"
	"testing"

	. "github.com/CramBL/mtgo-collection-manager/mtgogetter/pkg/mtgogetter"
)

// TODO: Mock HTTP requests: https://www.thegreatcodeadventure.com/mocking-http-requests-in-golang/
func TestDownloadBodyToBytes(t *testing.T) {
	got_body_bytes, err := DownloadBodyToBytes("https://raw.githubusercontent.com/CramBL/mtgo-collection-manager/main/LICENSE")
	if err != nil {
		t.Errorf("Error downloading body: %s", err)
	}

	log.Println("Got body len ==", len(got_body_bytes))
	if len(got_body_bytes) != 1072 {
		t.Errorf("Expected response body size of 1072 from DownloadBodyToBytes, got %d", len(got_body_bytes))
	}

	body_as_string := string(got_body_bytes)

	if body_as_string[0:11] != "MIT License" {
		t.Errorf("Expected body to start with 'MIT License', got %s", body_as_string[0:11])
	}
}

func TestRetryingDownload(t *testing.T) {
	attempts := 3
	delay_ms := 1
	function_download := func() ([]byte, error) {
		return DownloadBodyToBytes("https://raw.githubusercontent.com/CramBL/mtgo-collection-manager/main/LICENSE")
	}
	resp_body, err := Retry(attempts, delay_ms, function_download)
	if err != nil {
		t.Errorf("Error downloading body: %s", err)
	}

	if string(resp_body)[0:11] != "MIT License" {
		t.Errorf("Expected body to start with 'MIT License', got %s", string(resp_body)[0:11])
	}

}
