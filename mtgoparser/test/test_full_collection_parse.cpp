// NOLINTBEGIN
#if _MSC_VER && !__INTEL_COMPILER
// On MSVC: Disable warning C4702: unreachable code
//  This warning is generated for glaze@1.4.3 glaze\json\read.hpp l. 1574, 1577, 1580 & 1584
#pragma warning(disable : 4702)
#endif

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
using Catch::Matchers::ContainsSubstring;

#include "mtgoparser/goatbots.hpp"
#include "mtgoparser/io.hpp"
#include "mtgoparser/mtgo.hpp"
#include "mtgoparser/mtgo/history_aggregator.hpp"
#include "mtgoparser/scryfall.hpp"

#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <utility>
#include <vector>

// Used by the first section (small collection)
// Goes to top of project and into the shared 'test/test-data' directory
const auto path_goatbots_card_defs_small_5cards = "../../../test/test-data/goatbots/card-defs-small-5cards.json";
const auto path_goatbots_price_hist_small_5cards = "../../../test/test-data/goatbots/price-hist-small-5cards.json";
const auto path_mtgogetter_out_scryfall_small_5cards =
  "../../../test/test-data/mtgogetter-out/scryfall-small-5cards.json";
const auto path_trade_list_small_5cards = "../../../test/test-data/mtgo/Full Trade List-small-5cards.dek";


TEST_CASE("Parse small collection")
{
  SECTION("Small collection - 5 cards")
  {
    auto scryfall_vec = scryfall::ReadJsonVector(path_mtgogetter_out_scryfall_small_5cards);
    REQUIRE(scryfall_vec.has_value());
    CHECK(scryfall_vec.value().size() == 5);

    auto card_defs = goatbots::ReadJsonMap<goatbots::card_defs_map_t>(path_goatbots_card_defs_small_5cards);
    REQUIRE(card_defs.has_value());
    CHECK(card_defs.value().size() == 5);

    auto price_hist = goatbots::ReadJsonMap<goatbots::price_hist_map_t>(path_goatbots_price_hist_small_5cards);
    REQUIRE(price_hist.has_value());
    CHECK(price_hist.value().size() == 5);

    auto res_mtgo_cards = mtgo::xml::parse_dek_xml(path_trade_list_small_5cards);
    REQUIRE(res_mtgo_cards.has_value());
    auto mtgo_cards = std::move(res_mtgo_cards.value());
    CHECK(mtgo_cards.size() == 5);

    auto mtgo_collection = mtgo::Collection(std::move(mtgo_cards));
    REQUIRE(mtgo_collection.Size() == 5);

    mtgo_collection.ExtractGoatbotsInfo(card_defs.value(), price_hist.value());
    mtgo_collection.PrettyPrint();
    mtgo_collection.ExtractScryfallInfo(std::move(scryfall_vec.value()));
    mtgo_collection.PrettyPrint();

    CHECK(mtgo_collection.TotalCards() == 457);

    auto pretty_json_str = mtgo_collection.ToJsonPretty();
    CHECK(mtgo_collection == mtgo::Collection(pretty_json_str));
  }

  const auto path_mtgogetter_out_scryfall_full = "../../../test/test-data/mtgogetter-out/scryfall-20231002-full.json";
  const auto path_trade_list_medium_3000cards = "../../../test/test-data/mtgo/Full Trade List-medium-3000cards.dek";
  const auto path_goatbots_card_defs_full = "../../../test/test-data/goatbots/card-definitions-2023-10-02-full.json";
  const auto path_goatbots_price_hist_full = "../../../test/test-data/goatbots/price-history-2023-10-02-full.json";


  SECTION("Medium collection - 3000 cards")
  {
    auto scryfall_vec = scryfall::ReadJsonVector(path_mtgogetter_out_scryfall_full);
    REQUIRE(scryfall_vec.has_value());
    CHECK(scryfall_vec.value().size() == 43705);

    auto card_defs = goatbots::ReadJsonMap<goatbots::card_defs_map_t>(path_goatbots_card_defs_full);
    REQUIRE(card_defs.has_value());
    CHECK(card_defs.value().size() == 76070);

    auto price_hist = goatbots::ReadJsonMap<goatbots::price_hist_map_t>(path_goatbots_price_hist_full);
    REQUIRE(price_hist.has_value());
    CHECK(price_hist.value().size() == 76070);

    auto res_mtgo_cards = mtgo::xml::parse_dek_xml(path_trade_list_medium_3000cards);
    REQUIRE(res_mtgo_cards.has_value());
    auto mtgo_cards = std::move(res_mtgo_cards.value());
    CHECK(mtgo_cards.size() == 3000);

    auto mtgo_collection = mtgo::Collection(std::move(mtgo_cards));
    REQUIRE(mtgo_collection.Size() == 3000);

    mtgo_collection.ExtractGoatbotsInfo(card_defs.value(), price_hist.value());
    mtgo_collection.ExtractScryfallInfo(std::move(scryfall_vec.value()));
    mtgo_collection.PrettyPrint();

    auto pretty_json_str = mtgo_collection.ToJsonPretty();
    REQUIRE(mtgo_collection == mtgo::Collection(pretty_json_str));

    SECTION("Aggregate price history")
    {
      // Save the same collection to JSON twice with different timestamps
      // Then aggregate the two collections
      auto json_str = mtgo_collection.ToJson();
      auto json_str2 = mtgo_collection.ToJson();

      // Save to files
      const std::string fname{ "mtgo_cards_2023-11-05T152700Z" };
      const std::string fname2{ "mtgo_cards_2023-11-05T152800Z" };

      const std::string sub_dir{ "collection-history-full-collection-parse" };
      std::filesystem::create_directory(sub_dir);

      const std::filesystem::path fpath{ sub_dir + "/" + fname };
      const std::filesystem::path fpath2{ sub_dir + "/" + fname2 };
      INFO("Saving files to: " << fpath.string() << " and " << fpath2.string());
      INFO("Files extensions: " << fpath.extension().string() << " and " << fpath2.extension().string());

      {
        std::ofstream test_file(fpath);
        test_file << json_str;
        std::ofstream test_file2(fpath2);
        test_file2 << json_str2;
      }

      INFO("Saved files to: " << fpath.string() << " and " << fpath2.string());


      // Get the files with timestamps
      auto json_files = io_util::get_files_with_timestamp(sub_dir);
      INFO("Got files with timestamps: " << json_files[0].fpath_.string() << " and " << json_files[1].fpath_.string());
      CHECK(json_files[0].timestamp_ == "2023-11-05T152700Z");
      CHECK(json_files[1].timestamp_ == "2023-11-05T152800Z");

      // Take a copy of the most recent timestamp for the CSV-file suffix
      auto last_timestamp = json_files.at(1).timestamp_;

      // Deserialize the files
      auto new_json_str = io_util::read_to_str_buf(json_files[0].fpath_);
      auto new_json_str2 = io_util::read_to_str_buf(json_files[1].fpath_);

      // Deserialize the collections
      auto new_collection = mtgo::Collection(new_json_str);
      auto new_collection2 = mtgo::Collection(new_json_str2);
      CHECK(new_collection.Size() == 3000);
      CHECK(new_collection2.Size() == 3000);

      // Aggregate the collections
      auto cards = new_collection.TakeCards();

      // Transform the vector to a vector of CardHistory
      std::vector<mtgo::CardHistory> card_histories;
      card_histories.reserve(cards.size());
      for (auto &&card : cards) { card_histories.emplace_back(std::move(card)); }
      CHECK(card_histories.size() == 3000);

      auto collection_history = mtgo::CollectionHistory(std::move(card_histories), std::move(json_files[0].timestamp_));
      CHECK(collection_history.Size() == 3000);

      // Add the second collection to the aggregate
      collection_history.addCollectionPriceHistory(std::move(new_collection2), std::move(json_files[1].timestamp_));
      CHECK(collection_history.Size() == 3000);

      // Save the aggregate to a CSV file
      const std::string csv_fname{ "mtgo_cards_csv_" + last_timestamp + ".csv" };
      const std::filesystem::path csv_fpath{ sub_dir + "/" + csv_fname };
      INFO("Saving CSV file to: " << csv_fpath.string());

      {
        std::ofstream csv_file(csv_fpath);
        auto csv_string = collection_history.ToCsvStr();
        INFO("CSV string: " << csv_string);
        csv_file << collection_history.ToCsvStr();
        fmt::println("CSV string:\n{}", csv_string);
      }

      // Clean up by removing the files and directory
      std::filesystem::remove_all(sub_dir);

      SECTION("mtgo::CardAggregator::SaveAsCsvWithTimestamp")
      {
        // Create the sub_dir again and do the last steps but with the mtgo::CardAggregator::SaveAsCsvWithTimestamp
        // member function
        std::filesystem::create_directory(sub_dir);
        const std::string new_csv_fname{ "mtgo_cards" };
        std::filesystem::path new_csv_fpath{ sub_dir + "/" + new_csv_fname };
        INFO("Saving CSV file to: " << new_csv_fpath.string());

        collection_history.SaveAsCsvWithTimestamp(new_csv_fpath);
        CHECK(std::filesystem::exists(new_csv_fpath));

        // Check that the filename has the correct timestamp suffix
        INFO("CSV file name: " << new_csv_fpath.filename().string());
        CHECK(new_csv_fpath.filename().string() == "mtgo_cards_2023-11-05T152800Z.csv");
        CHECK(new_csv_fpath.extension().string() == ".csv");

        // Clean up by removing the files and directory
        std::filesystem::remove_all(sub_dir);
      }
    }
  }
}

// NOLINTEND