# MTGO Updater

## Purpose

Expose native Rust functions that makes using Go and C++ code easy.

## Get started
> Prefer using `task` from the project root to build and test MTGO Updater. e.g. `task mtgoupdater:build`

Build
```shell
cargo build
```

Test
>assumes `mtgoparser` was build in `Release` mode with `Ninja Multi-Config`
```shell
cargo test
```