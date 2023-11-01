
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
  - [Features? Make an issue if you have suggestions.](#features-make-an-issue-if-you-have-suggestions)
    - [Most recent demo](#most-recent-demo)
- [Contributing](#contributing)
  - [Quickstart](#quickstart)
    - [Unix-like (with Make)](#unix-like-with-make)
    - [Windows (with PowerShell)](#windows-with-powershell)
    - [Trouble shooting](#trouble-shooting)
      - [Compiling FLTK-rs on Linux](#compiling-fltk-rs-on-linux)
      - [Ubuntu](#ubuntu)
      - [Debian](#debian)


## Features? Make an issue if you have suggestions.
If you have a great idea, make a feature request via an issue, thanks!

### Most recent demo
![Demo](.github/most-recent-demo.gif)

# Contributing
There's scripts for building and testing all projects described in the [Quickstart](#quickstart) section below.

You're welcome to submit PRs or make issues.

If you're serious about starting a collaboration, send me a mail at `mbkj@tutamail.com`.

## Quickstart

### Unix-like (with Make)
A Makefile lets you build and test any or all of the subprojects.

Check versions of Rust/Go/C++ (also display minimum required versions) and more
```shell
make show-versions
```

Build all projects `order: MTGO Getter -> MTGO Parser -> MTGO Updater -> MTGO GUI`
```shell
make
```
Test all projects `order: MTGO Getter -> MTGO Parser -> MTGO Updater -> MTGO GUI`
```shell
make test
```
Build/test a single subproject with the `-{projectname}`-suffix e.g.
```shell
make test-mtgogetter
```

### Windows (with PowerShell)

A PowerShell script lets you build and test any or all of the subprojects in a manner similar to a Makefile.

Check versions of Rust/Go/C++ (also display minimum required versions) and more
```shell
.\wmake.ps1 show-versions
```

Build all projects `order: MTGO Getter -> MTGO Parser -> MTGO Updater -> MTGO GUI`
```shell
.\wmake.ps1
```
Test all projects `order: MTGO Getter -> MTGO Parser -> MTGO Updater -> MTGO GUI`
```shell
.\wmake.ps1 test
```
Build/test a single subproject with the `-{projectname}`-suffix e.g.
```shell
.\wmake.ps1 test-mtgogetter
```

### Trouble shooting
#### Compiling FLTK-rs on Linux
Compiling FLTK requires some development headers on Linux.
#### Ubuntu
Everything should be in [linux-fltk-deps.txt](linux-fltk-deps.txt) which the CI Linux runners install with `cat linux-fltk-deps.txt | xargs sudo apt-get -y install`.

Tested on Ubuntu 22.04.3 and Lubuntu 22.04.3.

#### Debian
When compiling with GCC you will need headers for statically linking with **libstdc++**, which are installable via **dnf** and are found as `libstdc++-static.<CPU architecture>` or simiar, e.g. `libstdc++-static.x86_64`.

Tested on CentOS Stream 9.