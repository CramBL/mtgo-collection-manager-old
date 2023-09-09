package mtgogetter

import (
	"log"
	"net/http"
	"time"

	"strings"

	"github.com/PuerkitoBio/goquery"
)

func GetGoatbotsDownloadPricesDoc() (*goquery.Document, error) {
	// Get the HTML document
	res, err := http.Get("https://www.goatbots.com/download-prices")
  	if err != nil {
   		log.Fatal(err)
 	}
	defer res.Body.Close()

	if res.StatusCode != 200 {
		log.Fatalf("status code error: %d %s", res.StatusCode, res.Status)
	}

	return goquery.NewDocumentFromReader(res.Body)
}

func ExtractGoatbotsPricesDate(prices_html_doc *goquery.Document) time.Time {
	// Find the date string within the HTML
	date_string := prices_html_doc.Find("li a.link").FilterFunction(func(i int, s *goquery.Selection) bool {
		return strings.HasPrefix(s.Text(), "Prices (2") // Time bomb, breaks in the year 3000 :)
	}).Text()

	// Extract the date from the string (assuming it's always in the same format)
	date_part := strings.Split(date_string, " ")[1] // Split by space and take the second part

	// Parse the date string
	date, err := time.Parse("2006-01-02", date_part)
	if err != nil {
		log.Fatal(err)
	}

	return date
}

func GetPricesDate() {
	// Get the HTML document
	prices_html_doc, err := GetGoatbotsDownloadPricesDoc()
	if err != nil {
	  log.Fatal(err)
	}

	// Find the date string within the HTML
	date := ExtractGoatbotsPricesDate(prices_html_doc)
	log.Println(date)
}