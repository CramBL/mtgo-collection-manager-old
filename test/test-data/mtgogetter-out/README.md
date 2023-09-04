# Format examples of MTGO Getter output

## TODO: Create single source of truth with protocol buffers (or something else?)

## Scryfall API

MTGO Getter outputs scryfall bulk data as a JSON-file containing an array of `ScryfallCard` (defined in MTGO Getter) containing the data that is deemed relevant or potentially relevant for MTGO Collection Manager as of now.

`scryfall-card.json` is an example of a serialized `ScryfallCard`.