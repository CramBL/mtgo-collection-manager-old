
<p align="center">
<img src="mtgogui/assets/logo-card-pile.png" alt="logo" width="150"/>
</p>
<h1 align="center">
MTGO Collection Manager
</h1>

## Purpose
To automate some tasks regarding effective management of [MTGO](https://www.mtgo.com/en/mtgo) collection, that are too cumbersome for anyone to actually do them manually.

# Table of contents
- [Table of contents](#table-of-contents)
  - [Features? Who knows, we'll see what I come up with, and how much time I have.](#features-who-knows-well-see-what-i-come-up-with-and-how-much-time-i-have)
    - [Feature Ideas:](#feature-ideas)
- [Contributing](#contributing)
  - [Quickstart](#quickstart)
    - [Unix-like (with Make)](#unix-like-with-make)
    - [Windows (with PowerShell)](#windows-with-powershell)


## Features? Who knows, we'll see what I come up with, and how much time I have.
If you have a great idea, make a feature request via an issue, thanks!
### Feature Ideas:

* **Price alerts** certain sites already have price alerts, but they are kind of crappy and hard to maintain. So better and smarter price alerts is a place to start.
* **Auto fetch users full MTGO collection** might be difficult. MTGO's local user files are a giant mess, it's solvable for sure, but might break quite often depending on how MTGO files are actually managed long term. Could be difficult to handle multiple accounts as well.
* **[A million data driven features]** like giving alerts when a card with a historically stable price suddenly spikes, and stuff like that.

# Contributing

This project is in a very early phase (the technology for GUI is still an open question), but build tools and CI are in a state where the practical aspects of making simple contributions should be fairly straightforward.

There's scripts for building and testing all projects described in the [Quickstart](#quickstart) section below.

If you're serious about starting a collaboration, send me a mail at `mbkj@tutamail.com`
## Quickstart

### Unix-like (with Make)
A Makefile lets you build and test any or all of the subprojects.

Check versions of Rust/Go/C++ (also display minimum required versions) and more
```shell
make show-versions
```

Build all projects `order: MTGO Getter -> MTGO Parser -> MTGO Updater`
```shell
make
```
Test all projects `order: MTGO Getter -> MTGO Parser -> MTGO Updater`
```shell
make test
```
Build/test a single subproject with the `_{projectname}`-suffix e.g.
```shell
make test-mtgogetter
```

### Windows (with PowerShell)

A PowerShell script lets you build and test any or all of the subprojects in a manner similar to a Makefile.

Check versions of Rust/Go/C++ (also display minimum required versions) and more
```shell
.\wmake.ps1 show-versions
```

Build all projects `order: MTGO Getter -> MTGO Parser -> MTGO Updater`
```shell
.\wmake.ps1
```
Test all projects `order: MTGO Getter -> MTGO Parser -> MTGO Updater`
```shell
.\wmake.ps1 test
```
Build/test a single subproject with the `-{projectname}`-suffix e.g.
```shell
.\wmake.ps1 test-mtgogetter
```