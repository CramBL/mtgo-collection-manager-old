package mtgogetter

import (
	"archive/zip"
	"bytes"
	"fmt"
	"io"
	"path/filepath"
	"time"

	"log"
	"net/http"
	"os"

	"github.com/spf13/cobra"
)

func DownloadBodyToBytes(url string) (respBody []byte, err error) {
	log.Println("Downloading from", url)

	resp, err := http.Get(url)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()

	if resp.StatusCode != 200 {
		return nil, fmt.Errorf("HTTP response code %d: %s", resp.StatusCode, http.StatusText(resp.StatusCode))
	}

	bodyAsBytes, err := io.ReadAll(resp.Body)
	if err != nil {
		return nil, err
	}

	return bodyAsBytes, nil
}

func UnzipBufAndWriteToDisk(byteSlice []byte) {
	reader, err := zip.NewReader(bytes.NewReader(byteSlice), int64(len(byteSlice)))
	if err != nil {
		log.Println("Error creating zip reader:", err)
		return
	}

	for _, file := range reader.File {
		log.Println("Extracting:", file.Name)

		// Open file from zip archive
		read_closer, err := file.Open()
		if err != nil {
			log.Println("Error opening file from zip:", err)
			continue
		}
		defer read_closer.Close()

		// Create file on disk for writing
		fd, err := os.OpenFile(file.Name, os.O_WRONLY|os.O_CREATE|os.O_TRUNC, file.Mode())
		if err != nil {
			log.Println("Error creating file:", err)
			continue
		}
		defer fd.Close()

		// Copy the file from the archive to the created file on disk
		_, err = io.Copy(fd, read_closer)
		if err != nil {
			log.Println("Error extracting file:", err)
		}
	}
}

func UnzipFromBytes(byteSlice []byte) (*zip.Reader, error) {
	reader, err := zip.NewReader(bytes.NewReader(byteSlice), int64(len(byteSlice)))
	if err != nil {
		return nil, err
	}
	return reader, nil
}

func FirstFileFromZipToDisk(fname string, zip_reader *zip.Reader) error {
	first_file_reader := zip_reader.File[0]
	log.Println("Extracting:", first_file_reader.Name, "and saving as", fname)

	first_file, err := first_file_reader.Open()
	if err != nil {
		return err
	}

	// Create file on disk for writing
	_, err = ReadCloserToDisk(first_file, fname)
	if err != nil {
		return err
	}
	return nil
}

func FirstFileFromZip(zip_reader *zip.Reader) (io.ReadCloser, error) {
	first_file_reader := zip_reader.File[0]

	first_file, err := first_file_reader.Open()
	if err != nil {
		return nil, err
	}

	return first_file, nil
}

func ReadCloserToStdout(read_closer io.ReadCloser) (int64, error) {
	written_bytes, err := io.Copy(os.Stdout, read_closer)
	if err != nil {
		return written_bytes, err
	}
	return written_bytes, nil
}

func ReadCloserToDisk(read_closer io.ReadCloser, fname string) (int64, error) {
	fd, err := os.OpenFile(fname, os.O_WRONLY|os.O_CREATE|os.O_TRUNC, 0777)
	if err != nil {
		return 0, err
	}
	defer fd.Close()

	written_bytes, err := io.Copy(fd, read_closer)
	if err != nil {
		return written_bytes, err
	}
	return written_bytes, nil
}

func ReadCloserToPath(read_closer io.ReadCloser, path string) (int64, error) {
	dir := filepath.Dir(path)
	err := os.MkdirAll(dir, 0777)
	if err != nil {
		return 0, err
	}
	fd, err := os.Create(path)
	if err != nil {
		return 0, err
	}
	defer fd.Close()

	written_bytes, err := io.Copy(fd, read_closer)
	if err != nil {
		return written_bytes, err
	}
	return written_bytes, nil
}

// Determine if the user wants to write to stdout or a file
func OutputIsStdout(cmd *cobra.Command) bool {
	is_save_as_set := cmd.Flag("save-as").Changed
	fname := cmd.Flag("save-as").Value.String()
	// Written as an AND statement to allow short-circuiting (with de morgan's law)
	return !(is_save_as_set && fname != "stdout")
}

// Let's a function that might fail retry a few times before giving up (such as file access)
// uses exponential backoff
func Retry[T any](attempts int, sleep_ms int, f func() (T, error)) (result T, err error) {
	for i := 0; i < attempts; i++ {
		if i > 0 {
			log.Println("retrying after error:", err)
			time.Sleep(time.Duration(sleep_ms) * time.Millisecond)
			sleep_ms = sleep_ms * 2 // exponential backoff
		}
		result, err = f()
		if err == nil {
			return result, nil
		}
	}
	return result, fmt.Errorf("after %d attempts, last error: %s", attempts, err)
}

// Function for filtering from a slice with a predicate, and returning a new slice
// https://stackoverflow.com/a/37563128
func Filter[T any](ss []T, test func(T) bool) (ret []T) {
	for _, s := range ss {
		if test(s) {
			ret = append(ret, s)
		}
	}
	return
}

// Function for filtering from a slice with a predicate, and returning a new slice that is preallocated
// adapted from: https://stackoverflow.com/a/37563128
func FilterPrealloc[T any](ss []T, test func(T) bool, prealloc int) []T {
	ret := make([]T, 0, prealloc)
	for _, s := range ss {
		if test(s) {
			ret = append(ret, s)
		}
	}
	return ret
}
