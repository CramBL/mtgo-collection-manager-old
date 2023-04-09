# MTGO Collection Manager
## Purpose
To automate some tasks regarding effective management of [MTGO](https://www.mtgo.com/en/mtgo) collection, that are too cumbersome for anyone to actually do them manually.

## Features? Who knows, we'll see what I come up with, and how much time I have.
If you have a great idea, make a feature request via an issue, thanks!
### Feature Ideas:

* **Price alerts** certain sites already have price alerts, but they are kind of crappy and hard to maintain. So better and smarter price alerts is a place to start.
* **Auto fetch users full MTGO collection** might be difficult. MTGO's local user files are a giant mess, it's solvable for sure, but might break quite often depending on how MTGO files are actually managed long term. Could be difficult to handle multiple accounts as well.
* **[A million data driven features]** like giving alerts when a card with a historically stable price suddenly spikes, and stuff like that.

## Currently running MTGO Collection Manager assumes
* The directory it runs from contains a [chrome driver](https://chromedriver.chromium.org/downloads) called `chromedriver.exe`
* `Python 3.10` is installed with [Selenium](https://pypi.org/project/selenium/)

These assumption will change over time, and some will become much less strict such as the Python requirement, but for now user friendliness is not a priority.

## Thanks
To [goatbots.com](https://www.goatbots.com/) for providing the data that makes this project possible. All price tracking is based on Goatbots' prices, and they offer by far the most competitive prices, and as such it is only a tiny tiny loss to not track prices from other vendors.