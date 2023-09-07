package mtgogetter

import (
	"archive/zip"
	"bytes"
	"io"

	"log"
	"net/http"
	"os"

	"github.com/spf13/cobra"
)

func DownloadBodyToBytes(url string) (respBody []byte) {
	log.Println("Downloading from", url)

	resp, err := http.Get(url)
	if err != nil {
		log.Fatal(err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != 200 {
		log.Fatalln("Get returned:", resp.StatusCode, http.StatusText(resp.StatusCode))
	}

	bodyAsBytes, err := io.ReadAll(resp.Body)
	if err != nil {
		log.Fatal(err)
	}

	return bodyAsBytes
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

func UnzipFromBytes(byteSlice []byte) *zip.Reader {
	reader, err := zip.NewReader(bytes.NewReader(byteSlice), int64(len(byteSlice)))
	if err != nil {
		log.Fatalln("Error creating zip reader:", err)
	}
	return reader
}

func FirstFileFromZipToDisk(fname string, zip_reader *zip.Reader) {
	first_file_reader := zip_reader.File[0]
	log.Println("Extracting:", first_file_reader.Name, "and saving as", fname)

	first_file, err := first_file_reader.Open()
	if err != nil {
		log.Fatalln("Error opening first file from zip archive: ", err)
	}

	// Create file on disk for writing
	ReadCloserToDisk(first_file, fname)
	if err != nil {
		log.Fatalln("Error writing file:", err)
	}
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

// Determine if the user wants to write to stdout or a file
func OutputIsStdout(cmd *cobra.Command) bool {
	is_save_as_set := cmd.Flag("save-as").Changed
	fname := cmd.Flag("save-as").Value.String()
	// Written as an AND statement to allow short-circuiting (with de morgan's law)
	return !(is_save_as_set && fname != "stdout")
}