#pragma once

#include "goatbots.hpp"
#include "io.hpp"
#include <iostream>

void example() {
  using goatbots::card_defs_map_t;
  using goatbots::CardDefinition;
  using goatbots::price_hist_map_t;

  std::optional<card_defs_map_t> cards = goatbots::ReadJsonMap<card_defs_map_t>(
      "./test/test-data/card-defs-small.json");
  if (!cards.has_value()) {
    // Error: ReadJsonMap() failed
  }
  price_hist_map_t prices = goatbots::ReadJsonMap<price_hist_map_t>(
                                "./test/test-data/price-hist-small.json")
                                .value();

  for (auto &&e : cards.value()) {
    if (e.second.name == "Black Lotus") {

      std::cout << e.first << " " << e.second.name
                << " price = " << prices.at(e.first) << " \n";
    }
  }
}

int main() {
  example();

  return 0;
}