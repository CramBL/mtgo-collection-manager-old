package mtgogetter

import (
	"archive/zip"
	"bytes"
	"io"
	"log"
	"net/http"
	"os"
)

func DownloadToBuf(url string) (buffer bytes.Buffer) {
	log.Println("Downloading from", url)

	resp, err := http.Get(url)
	if err != nil {
		log.Println("Error downloading file:", err)
		return
	}
	defer resp.Body.Close()
	// Read the file content into a buffer
	var buf bytes.Buffer
	_, err = io.Copy(&buf, resp.Body)
	if err != nil {
		log.Println("Error reading file content:", err)
		return
	}
	return buf
}

func UnzipBufAndWriteToDisk(buf bytes.Buffer) {
	reader, err := zip.NewReader(bytes.NewReader(buf.Bytes()), int64(buf.Len()))
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

func UnzipFromBuf(buf bytes.Buffer) *zip.Reader {
	reader, err := zip.NewReader(bytes.NewReader(buf.Bytes()), int64(buf.Len()))
	if err != nil {
		log.Fatalln("Error creating zip reader:", err)
	}
	return reader
}

func FirstFileFromZipToDisk(fname string, zip_reader *zip.Reader) {
	first_file_reader := zip_reader.File[0]
	log.Println("Extracting:", first_file_reader.Name)

	first_file, err := first_file_reader.Open()
	if err != nil {
		log.Fatalln("Error opening first file from zip archive: ", err)
	}

	// Create file on disk for writing
	fd, err := os.OpenFile(fname, os.O_WRONLY|os.O_CREATE|os.O_TRUNC, first_file_reader.Mode())
	if err != nil {
		log.Fatalln("Error creating file:", err)
	}
	defer fd.Close()

	// Copy the file from the archive to the created file on disk
	_, err = io.Copy(fd, first_file)
	if err != nil {
		log.Fatalln("Error extracting file:", err)
	}
}
