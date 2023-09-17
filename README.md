# MTGO Collection Manager
## Purpose
To automate some tasks regarding effective management of [MTGO](https://www.mtgo.com/en/mtgo) collection, that are too cumbersome for anyone to actually do them manually. 

### Personal purpose
Learn new technologies, practice Rust, Go, C++.

# Table of contents
- [MTGO Collection Manager](#mtgo-collection-manager)
  - [Purpose](#purpose)
    - [Personal purpose](#personal-purpose)
- [Table of contents](#table-of-contents)
  - [Features? Who knows, we'll see what I come up with, and how much time I have.](#features-who-knows-well-see-what-i-come-up-with-and-how-much-time-i-have)
    - [Feature Ideas:](#feature-ideas)
  - [Quickstart](#quickstart)


## Features? Who knows, we'll see what I come up with, and how much time I have.
If you have a great idea, make a feature request via an issue, thanks!
### Feature Ideas:

* **Price alerts** certain sites already have price alerts, but they are kind of crappy and hard to maintain. So better and smarter price alerts is a place to start.
* **Auto fetch users full MTGO collection** might be difficult. MTGO's local user files are a giant mess, it's solvable for sure, but might break quite often depending on how MTGO files are actually managed long term. Could be difficult to handle multiple accounts as well.
* **[A million data driven features]** like giving alerts when a card with a historically stable price suddenly spikes, and stuff like that.

## Quickstart
A Makefile lets you build and test any or all of the subprojects.

Check versions of Rust/Go/C++ (also display minimum required versions)
```shell
make show_versions
```

Build all projects `order: MTGO Getter -> MTGO Parser -> MTGO Updater`
```shell
make
```
Test all projects `order: MTGO Getter -> MTGO Parser -> MTGO Updater`
```shell
make test
```
Build/test a single subproject with the `_{project-name}`-suffix e.g. 
```shell
make test_mtgogetter
```