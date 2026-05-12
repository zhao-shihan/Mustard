// Copyright (C) 2020-2026  Shihan Zhao and contributors
//
// This file is part of Mustard, an offline software framework for HEP experiments.
//
// Mustard is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// Mustard is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// Mustard. If not, see <https://www.gnu.org/licenses/>.

#include "Mustard/CLI/MonteCarloCLI.h++"
#include "Mustard/Data/Container/ArcTupleVector.h++"
#include "Mustard/Data/Model.h++"
#include "Mustard/Data/Object/Tuple.h++"
#include "Mustard/Data/Processing/RDFEventReader.h++"
#include "Mustard/Data/Processing/RNTupleWriter.h++"
#include "Mustard/Data/Processing/TTreeWriter.h++"
#include "Mustard/Env/MonteCarloEnv.h++"
#include "Mustard/IO/File.h++"
#include "Mustard/IO/PrettyLog.h++"

#include "TestingSingleDataModel.h++"

#include "CLHEP/Random/RandGaussQ.h"
#include "CLHEP/Random/RandPoissonQ.h"
#include "CLHEP/Random/RandomEngine.h"

#include "ROOT/RDataFrame.hxx"

#include "muc/ceta_string"
#include "muc/utility"

#include "fmt/format.h"

#include <array>
#include <cstdint>
#include <cstdlib>
#include <iterator>
#include <memory>
#include <optional>
#include <source_location>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

auto main(int argc, char* argv[]) -> int {
    Mustard::CLI::MonteCarloCLI<> cli;
    cli->add_argument("-n", "--n-event").help("Number of events to generate").default_value(1000000ull).required().nargs(1).scan<'i', unsigned long long>();
    Mustard::Env::MonteCarloEnv<256> env{argc, argv, cli};
    auto& rng{*CLHEP::HepRandom::getTheEngine()};

    const auto nEvent{gsl::narrow<gsl::index>(cli->get<unsigned long long>("--n-event"))};
    constexpr auto fileName{"test_single_data_event_io.root"};
    constexpr auto ntupleName{"RNTuple"};
    constexpr auto treeName{"TTree"};

    const auto fail{[](std::string_view message, const std::source_location& location = std::source_location::current()) {
        Mustard::PrintError(message, location);
        return EXIT_FAILURE;
    }};

    gtl::vector<muc::array2i> eventIDAndEntryCount;
    eventIDAndEntryCount.reserve(nEvent);
    for (int evtID{}; ssize(eventIDAndEntryCount) < nEvent; ++evtID) {
        const auto entryCount{static_cast<int>(CLHEP::RandPoissonQ::shoot(&rng, 2))};
        if (entryCount == 0) {
            continue;
        }
        eventIDAndEntryCount.push_back({evtID, entryCount});
    }

    // Shuffle events to create some disorder in event order.
    constexpr auto nShuffle{static_cast<gsl::index>(10000)};
    for (gsl::index i{}; i < nEvent; i += nShuffle) {
        const auto iBegin{i};
        const auto iEnd{std::min(i + nShuffle, nEvent)};
        std::ranges::subrange target{eventIDAndEntryCount.begin() + iBegin, eventIDAndEntryCount.begin() + iEnd};
        std::ranges::shuffle(target, env.RandomEngine());
    }

    // Generate entries for each event
    Mustard::Data::ArcTupleVector<TestingModel> flatEntryData;
    std::vector<Mustard::Data::ArcTupleVector<TestingModel>> expectedEventData(nEvent);
    for (gsl::index i{}; i < nEvent; ++i) {
        const auto [eventID, entryCount]{eventIDAndEntryCount[i]};
        for (gsl::index entryIdx{}; entryIdx < entryCount; ++entryIdx) {
            auto entry{MakeEntry(eventID, entryIdx)};
            flatEntryData.emplace_back(entry);
            expectedEventData[i].emplace_back(std::move(entry));
        }
    }

    {
        Mustard::File<TFile> outputFile{fileName, "RECREATE"};

        const auto fillWriter{[&]<typename TWriter>(TWriter& writer, std::string_view writerName) -> int {
            writer.Fill(flatEntryData[0]);

            auto arcEntry{Mustard::Data::MakeArcTuple<TestingModel>(*flatEntryData[1])};
            writer.Fill(std::move(arcEntry));
            writer.Flush();

            Mustard::Data::ArcTupleVector<TestingModel> batch;
            batch.reserve(flatEntryData.size() - 2);
            for (gsl::index i{2}; i < ssize(flatEntryData); ++i) {
                batch.emplace_back(flatEntryData[i]);
            }
            writer.Fill(std::move(batch));

            if (writer.NEntry() != ssize(flatEntryData)) {
                return fail(fmt::format("{} entry count mismatch after Fill/Flush. expected={}, actual={}",
                                        writerName, flatEntryData.size(), writer.NEntry()));
            }
            return EXIT_SUCCESS;
        }};

        Mustard::Data::RNTupleWriter<TestingModel> ntupleWriter{ntupleName};
        if (fillWriter(ntupleWriter, "RNTupleWriter") != EXIT_SUCCESS) {
            return EXIT_FAILURE;
        }

        Mustard::Data::TTreeWriter<TestingModel> treeWriter{treeName};
        if (fillWriter(treeWriter, "TTreeWriter") != EXIT_SUCCESS) {
            return EXIT_FAILURE;
        }
    }

    const auto checkEvent{[&](const Mustard::Data::Tuple<TestingModel>& actual,
                              const Mustard::Data::Tuple<TestingModel>& expectedEntry,
                              std::string_view sourceName) -> int {
        if (actual == expectedEntry) {
            return EXIT_SUCCESS;
        }
        Mustard::PrintError(fmt::format("{} entry mismatch details:", sourceName));
        std::size_t mismatchCount{};
        const auto logMismatch{[&]<muc::ceta_string AName>() {
            if (Get<AName>(actual) != Get<AName>(expectedEntry)) {
                ++mismatchCount;
                Mustard::PrintError(fmt::format("  - field '{}' mismatch", AName.sv()));
            }
        }};

        logMismatch.operator()<"EvtID">();
        logMismatch.operator()<"b">();
        logMismatch.operator()<"ch">();
        logMismatch.operator()<"u8">();
        logMismatch.operator()<"i16">();
        logMismatch.operator()<"u16">();
        logMismatch.operator()<"i32">();
        logMismatch.operator()<"u32">();
        logMismatch.operator()<"i64">();
        logMismatch.operator()<"u64">();
        logMismatch.operator()<"f32">();
        logMismatch.operator()<"f64">();
        logMismatch.operator()<"str">();
        logMismatch.operator()<"vi32">();
        logMismatch.operator()<"vf64">();
        logMismatch.operator()<"vstr">();
        logMismatch.operator()<"au16">();
        logMismatch.operator()<"pi32f">();
        logMismatch.operator()<"tu8str">();

        if (mismatchCount == 0) {
            Mustard::PrintError("  - no field-level mismatch found although tuple comparison failed");
        }
        return fail(fmt::format("{} entry content mismatch", sourceName));
    }};

    const auto validateWithRDFEventReader{[&](std::string_view dataName, std::string_view sourceName) -> int {
        ROOT::RDataFrame rdf{dataName, fileName};
        Mustard::Data::RDFEventReader<int, TestingModel> reader{rdf, "EvtID"};
        const auto count{reader.NEvent()};

        // Build filtered list of non‑empty expected events
        std::vector<Mustard::Data::ArcTupleVector<TestingModel>> expectedNonEmpty;
        for (auto&& event : std::as_const(expectedEventData)) {
            if (not event.empty()) {
                expectedNonEmpty.push_back(event);
            }
        }

        if (count != ssize(expectedNonEmpty)) {
            return fail(fmt::format("{} event count mismatch. expected={}, actual={}", sourceName, expectedNonEmpty.size(), count));
        }

        const auto data{reader.ReadNext(count)};
        if (data.size() != expectedNonEmpty.size()) {
            return fail(fmt::format("{} read size mismatch. expected={}, actual={}", sourceName, expectedNonEmpty.size(), data.size()));
        }

        for (gsl::index i{}; i < ssize(data); ++i) {
            if (data[i].size() != expectedNonEmpty[i].size()) {
                return fail(fmt::format("{} event {} entry count mismatch. expected={}, actual={}",
                                        sourceName, i, expectedNonEmpty[i].size(), data[i].size()));
            }
            for (gsl::index j{}; j < ssize(data[i]); ++j) {
                if (data[i][j] == nullptr) {
                    return fail(fmt::format("{} returned null entry at event {}, entry {}", sourceName, i, j));
                }
                if (checkEvent(*data[i][j], *expectedNonEmpty[i][j], sourceName) != EXIT_SUCCESS) {
                    return EXIT_FAILURE;
                }
            }
        }
        return EXIT_SUCCESS;
    }};

    if (validateWithRDFEventReader(ntupleName, "RDFEventReader(RNTuple)") != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }
    if (validateWithRDFEventReader(treeName, "RDFEventReader(TTree)") != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    // Build filtered list of non-empty expected events (needed for Reset test)
    std::vector<Mustard::Data::ArcTupleVector<TestingModel>> expectedNonEmpty;
    for (auto&& event : std::as_const(expectedEventData)) {
        if (not event.empty()) {
            expectedNonEmpty.push_back(event);
        }
    }

    // Helper to verify an event matches expected
    const auto verifyEvent{[&](gsl::index evtIdx, const auto& actualEvent, std::string_view sourceName) -> int {
        if (actualEvent.size() != expectedNonEmpty[evtIdx].size()) {
            Mustard::PrintError(fmt::format("{} event {} entry count mismatch. expected={}, actual={}",
                                            sourceName, evtIdx, expectedNonEmpty[evtIdx].size(), actualEvent.size()));
            return fail(fmt::format("{} event {} entry count mismatch", sourceName, evtIdx));
        }
        for (gsl::index j{}; j < ssize(actualEvent); ++j) {
            if (actualEvent[j] == nullptr) {
                return fail(fmt::format("{} returned null entry at event {}, entry {}", sourceName, evtIdx, j));
            }
            if (checkEvent(*actualEvent[j], *expectedNonEmpty[evtIdx][j], sourceName) != EXIT_SUCCESS) {
                return EXIT_FAILURE;
            }
        }
        return EXIT_SUCCESS;
    }};

    // Test Reset, ReadNext, SkipNext, and Exhaust
    const auto testEventReaderReset{[&](std::string_view dataName, std::string_view sourceName) -> int {
        ROOT::RDataFrame rdf{dataName, fileName};
        Mustard::Data::RDFEventReader<int, TestingModel> reader{rdf, "EvtID"};

        if (reader.NEvent() != ssize(expectedNonEmpty)) {
            return fail(fmt::format("{} event count mismatch before reset. expected={}, actual={}",
                                    sourceName, expectedNonEmpty.size(), reader.NEvent()));
        }

        reader.Exhaust();
        if (not reader.Exhausted()) {
            return fail(fmt::format("{} not exhausted after Exhaust()", sourceName));
        }

        reader.Reset();

        if (reader.NEvent() != ssize(expectedNonEmpty)) {
            return fail(fmt::format("{} event count mismatch after reset. expected={}, actual={}",
                                    sourceName, expectedNonEmpty.size(), reader.NEvent()));
        }

        // ReadNext(): single event
        auto event0{reader.ReadNext()};
        if (verifyEvent(0, event0, sourceName) != EXIT_SUCCESS) {
            return EXIT_FAILURE;
        }

        // ReadNext(3): batch of events
        auto batch{reader.ReadNext(3)};
        if (batch.size() != 3) {
            return fail(fmt::format("{} ReadNext(3) size mismatch. expected=3, actual={}",
                                    sourceName, batch.size()));
        }
        for (gsl::index i{}; i < 3; ++i) {
            if (verifyEvent(1 + i, batch[i], sourceName) != EXIT_SUCCESS) {
                return EXIT_FAILURE;
            }
        }

        // SkipNext(): skip 1 event
        reader.SkipNext();

        // SkipNext(3): skip 3 events
        reader.SkipNext(3);

        // Read one more event (should be at event index 1 + 3 + 1 + 3 = 8)
        auto event8{reader.ReadNext()};
        if (verifyEvent(8, event8, sourceName) != EXIT_SUCCESS) {
            return EXIT_FAILURE;
        }

        // Exhaust the rest
        reader.Exhaust();
        if (not reader.Exhausted()) {
            return fail(fmt::format("{} not exhausted after Exhaust()", sourceName));
        }

        return EXIT_SUCCESS;
    }};

    if (testEventReaderReset(ntupleName, "RDFEventReader+Reset(RNTuple)") != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }
    if (testEventReaderReset(treeName, "RDFEventReader+Reset(TTree)") != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
