# MTGO Getter
## Purpose
Simple command-line utility that allows quickly and efficiently downloading card data from various sources.

## Purpose in MTGO Collection Manager
Provide a cross-platform binary with batteries included downloading features for exactly what is needed in the MTGO collection manager.

Secondary purpose is to learn basic but idiomatic Go.

## Get started
> Prefer using `task` from the project root to build and test MTGO Getter. e.g. `task mtgogetter:build`

1. [Install Go](https://go.dev/doc/install)
2. run `go run . --help` this should compile and run `mtgogetter` and display the help text
3. run tests with `go test ./...`