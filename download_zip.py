""" Get the latest card prices from goatbots.com """
import time
import os
from selenium import webdriver
from selenium.webdriver.chrome.service import Service
from selenium.webdriver.common.by import By


def download_prices():
    """ Download the latest card prices from goatbots.com """
    current_dir_path = os.path.dirname(os.path.realpath(__file__))
    path_to_chrome_drv = os.path.join(current_dir_path, 'chromedriver.exe')
    # Set up the Chrome driver
    # replace with path to your chromedriver executable
    service = Service(
        path_to_chrome_drv)
    driver = webdriver.Chrome(service=service)

    # Navigate to the URL of the file you want to download
    url = 'https://www.goatbots.com/download-prices'
    driver.get(url)

    todays_card_prices_download = driver.find_element(
        By.XPATH, '/html/body/main/ul[1]/li[2]/a')

    todays_card_prices_download.click()

    # Wait for the file to download
    # adjust the sleep time to allow enough time for the file to download
    time.sleep(1)

    # Also download the card definitions if they are not already present
    is_card_defs_present = False
    path_to_card_defs = os.path.join(current_dir_path,
                                     'managed-files')
    for i in os.listdir(path_to_card_defs):
        if i.startswith('card-definitions') and i.is_file():
            is_card_defs_present = True
            break

    if is_card_defs_present is False:
        download_card_definitions(driver)

    driver.quit()


def download_card_definitions(web_driver: webdriver.Chrome):
    """ Download the latest card definitions from goatbots.com """
    # Click the download button or link
    card_definitions_download = web_driver.find_element(
        By.XPATH, '/html/body/main/ul[1]/li[1]/a')

    card_definitions_download.click()

    # Wait for the file to download
    # adjust the sleep time to allow enough time for the file to download
    time.sleep(1)


if __name__ == "__main__":
    download_prices()
