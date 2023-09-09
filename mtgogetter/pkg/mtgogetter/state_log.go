package mtgogetter

import "time"

type goatbots struct {
    Updated_at  time.Time
}

type scryfall struct {
    Updated_at  time.Time
}

type StateLog struct {
    Title string
    Goatbots    goatbots `toml:"goatbots"`
    Scryfall    scryfall `toml:"scryfall"`
}