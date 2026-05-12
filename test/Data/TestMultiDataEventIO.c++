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

#include "TestingMultiDataModel.h++"

#include "CLHEP/Random/RandGaussQ.h"
#include "CLHEP/Random/RandPoissonQ.h"
#include "CLHEP/Random/RandomEngine.h"

#include "ROOT/RDataFrame.hxx"

#include "gtl/phmap.hpp"
#include "gtl/vector.hpp"

#include "muc/array"
#include "muc/ceta_string"
#include "muc/utility"

#include "fmt/format.h"

#include <array>
#include <cstdint>
#include <cstdlib>
#include <iterator>
#include <memory>
#include <random>
#include <source_location>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

auto main(int argc, char* argv[]) -> int {
    Mustard::CLI::MonteCarloCLI<> cli;
    cli->add_argument("-a", "--n-event-a").help("Number of events for TestingModelA").default_value(1000000ull).required().nargs(1).scan<'i', unsigned long long>();
    cli->add_argument("-b", "--n-event-b").help("Number of events for TestingModelB").default_value(1500000ull).required().nargs(1).scan<'i', unsigned long long>();
    cli->add_argument("-c", "--n-event-c").help("Number of events for TestingModelC").default_value(800000ull).required().nargs(1).scan<'i', unsigned long long>();
    Mustard::Env::MonteCarloEnv<256> env{argc, argv, cli};
    auto& rng{env.CLHEPRandomEngine()};

    const auto nEventA{gsl::narrow<gsl::index>(cli->get<unsigned long long>("--n-event-a"))};
    const auto nEventB{gsl::narrow<gsl::index>(cli->get<unsigned long long>("--n-event-b"))};
    const auto nEventC{gsl::narrow<gsl::index>(cli->get<unsigned long long>("--n-event-c"))};
    constexpr auto fileName{"test_multi_data_event_io.root"};
    constexpr auto ntupleNameA{"RNTupleA"};
    constexpr auto ntupleNameB{"RNTupleB"};
    constexpr auto ntupleNameC{"RNTupleC"};
    constexpr auto treeNameA{"TTreeA"};
    constexpr auto treeNameB{"TTreeB"};
    constexpr auto treeNameC{"TTreeC"};

    const auto fail{[](std::string_view message, const std::source_location& location = std::source_location::current()) {
        Mustard::PrintError(message, location);
        return EXIT_FAILURE;
    }};

    // Generate expected data for each model
    std::vector<Mustard::Data::ArcTupleVector<TestingModelA>> expectedEventDataA(nEventA);
    std::vector<Mustard::Data::ArcTupleVector<TestingModelB>> expectedEventDataB(nEventB);
    std::vector<Mustard::Data::ArcTupleVector<TestingModelC>> expectedEventDataC(nEventC);
    Mustard::Data::ArcTupleVector<TestingModelA> flatEntryDataA;
    Mustard::Data::ArcTupleVector<TestingModelB> flatEntryDataB;
    Mustard::Data::ArcTupleVector<TestingModelC> flatEntryDataC;

    const auto generateModel{[&](auto nEvent, auto& expectedEventData, auto& flatEntryData, auto makeEntry) {
        gtl::vector<muc::array2i> eventIDAndEntryCount;
        eventIDAndEntryCount.reserve(nEvent);
        for (int evtID{}; ssize(eventIDAndEntryCount) < nEvent; ++evtID) {
            const auto entryCount{static_cast<int>(CLHEP::RandPoissonQ::shoot(&rng, 2))};
            if (entryCount == 0) {
                continue;
            }
            eventIDAndEntryCount.push_back({evtID, entryCount});
        }
        // Shuffle events to create some disorder in event order, but still maintain some locality.
        constexpr auto nShuffle{static_cast<gsl::index>(100)};
        for (gsl::index i{}; i < nEvent; i += nShuffle) {
            const auto iBegin{i};
            const auto iEnd{std::min(i + nShuffle, nEvent)};
            std::ranges::subrange target{eventIDAndEntryCount.begin() + iBegin, eventIDAndEntryCount.begin() + iEnd};
            std::ranges::shuffle(target, env.RandomEngine());
        }
        // Generate entries for each event
        for (gsl::index i{}; i < nEvent; ++i) {
            const auto [eventID, entryCount]{eventIDAndEntryCount[i]};
            for (gsl::index entryIdx{}; entryIdx < entryCount; ++entryIdx) {
                auto entry{makeEntry(eventID, entryIdx)};
                flatEntryData.emplace_back(entry);
                expectedEventData[i].emplace_back(std::move(entry));
            }
        }
    }};

    generateModel(nEventA, expectedEventDataA, flatEntryDataA, MakeEntryA);
    generateModel(nEventB, expectedEventDataB, flatEntryDataB, MakeEntryB);
    generateModel(nEventC, expectedEventDataC, flatEntryDataC, MakeEntryC);

    // Write data using RNTupleWriter and TTreeWriter
    {
        Mustard::File<TFile> outputFile{fileName, "RECREATE"};

        const auto fillWriter{[&]<typename Model>(
                                  auto& writer, const Mustard::Data::ArcTupleVector<Model>& flatEntryData,
                                  std::string_view writerName) -> int {
            writer.Fill(flatEntryData[0]);

            auto arcEntry{Mustard::Data::MakeArcTuple<Model>(*flatEntryData[1])};
            writer.Fill(std::move(arcEntry));
            writer.Flush();

            Mustard::Data::ArcTupleVector<Model> batch;
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

        Mustard::Data::RNTupleWriter<TestingModelA> ntupleWriterA{ntupleNameA};
        if (fillWriter(ntupleWriterA, flatEntryDataA, "RNTupleWriter(TestingModelA)") != EXIT_SUCCESS) {
            return EXIT_FAILURE;
        }

        Mustard::Data::RNTupleWriter<TestingModelB> ntupleWriterB{ntupleNameB};
        if (fillWriter(ntupleWriterB, flatEntryDataB, "RNTupleWriter(TestingModelB)") != EXIT_SUCCESS) {
            return EXIT_FAILURE;
        }

        Mustard::Data::RNTupleWriter<TestingModelC> ntupleWriterC{ntupleNameC};
        if (fillWriter(ntupleWriterC, flatEntryDataC, "RNTupleWriter(TestingModelC)") != EXIT_SUCCESS) {
            return EXIT_FAILURE;
        }

        Mustard::Data::TTreeWriter<TestingModelA> treeWriterA{treeNameA};
        if (fillWriter(treeWriterA, flatEntryDataA, "TTreeWriter(TestingModelA)") != EXIT_SUCCESS) {
            return EXIT_FAILURE;
        }

        Mustard::Data::TTreeWriter<TestingModelB> treeWriterB{treeNameB};
        if (fillWriter(treeWriterB, flatEntryDataB, "TTreeWriter(TestingModelB)") != EXIT_SUCCESS) {
            return EXIT_FAILURE;
        }

        Mustard::Data::TTreeWriter<TestingModelC> treeWriterC{treeNameC};
        if (fillWriter(treeWriterC, flatEntryDataC, "TTreeWriter(TestingModelC)") != EXIT_SUCCESS) {
            return EXIT_FAILURE;
        }
    }

    // Verification functions
    const auto checkEntryA{[&](const Mustard::Data::Tuple<TestingModelA>& actual,
                               const Mustard::Data::Tuple<TestingModelA>& expectedEntry,
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
        logMismatch.operator()<"i32">();
        logMismatch.operator()<"f64">();
        logMismatch.operator()<"str">();
        logMismatch.operator()<"b">();
        logMismatch.operator()<"ch">();
        logMismatch.operator()<"i64">();
        logMismatch.operator()<"u32">();
        logMismatch.operator()<"f32">();
        logMismatch.operator()<"vi32">();
        if (mismatchCount == 0) {
            Mustard::PrintError("  - no field-level mismatch found although tuple comparison failed");
        }
        return fail(fmt::format("{} entry content mismatch", sourceName));
    }};

    const auto checkEntryB{[&](const Mustard::Data::Tuple<TestingModelB>& actual,
                               const Mustard::Data::Tuple<TestingModelB>& expectedEntry,
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
        logMismatch.operator()<"f32">();
        logMismatch.operator()<"b">();
        logMismatch.operator()<"vi">();
        logMismatch.operator()<"vf64">();
        logMismatch.operator()<"vstr">();
        logMismatch.operator()<"u8">();
        logMismatch.operator()<"i16">();
        logMismatch.operator()<"pf32f64">();
        logMismatch.operator()<"tu16bstr">();
        if (mismatchCount == 0) {
            Mustard::PrintError("  - no field-level mismatch found although tuple comparison failed");
        }
        return fail(fmt::format("{} entry content mismatch", sourceName));
    }};

    const auto checkEntryC{[&](const Mustard::Data::Tuple<TestingModelC>& actual,
                               const Mustard::Data::Tuple<TestingModelC>& expectedEntry,
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
        logMismatch.operator()<"a3f64">();
        logMismatch.operator()<"p11">();
        logMismatch.operator()<"a4u32">();
        logMismatch.operator()<"str">();
        logMismatch.operator()<"i64">();
        logMismatch.operator()<"f64">();
        logMismatch.operator()<"tbchi32">();
        if (mismatchCount == 0) {
            Mustard::PrintError("  - no field-level mismatch found although tuple comparison failed");
        }
        return fail(fmt::format("{} entry content mismatch", sourceName));
    }};

    // Validate with RDFEventReader for each model
    const auto validateWithRDFEventReader{[&]<typename Model>(std::string_view dataName,
                                                              const std::vector<Mustard::Data::ArcTupleVector<Model>>& expectedEventData,
                                                              auto&& checkEntry, std::string_view sourceName) -> int {
        ROOT::RDataFrame rdf{dataName, fileName};
        Mustard::Data::RDFEventReader<int, Model> reader{rdf, "EvtID"};
        const auto count{reader.NEvent()};

        // Build filtered list of non-empty expected events
        std::vector<Mustard::Data::ArcTupleVector<Model>> expectedNonEmpty;
        for (auto& event : std::as_const(expectedEventData)) {
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
                if (checkEntry(*data[i][j], *expectedNonEmpty[i][j], sourceName) != EXIT_SUCCESS) {
                    return EXIT_FAILURE;
                }
            }
        }
        return EXIT_SUCCESS;
    }};

    // Run validations for each model and each storage type
    if (validateWithRDFEventReader(ntupleNameA, expectedEventDataA, checkEntryA, "RDFEventReader(RNTuple TestingModelA)") != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }
    if (validateWithRDFEventReader(ntupleNameB, expectedEventDataB, checkEntryB, "RDFEventReader(RNTuple TestingModelB)") != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }
    if (validateWithRDFEventReader(ntupleNameC, expectedEventDataC, checkEntryC, "RDFEventReader(RNTuple TestingModelC)") != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }
    if (validateWithRDFEventReader(treeNameA, expectedEventDataA, checkEntryA, "RDFEventReader(TTree TestingModelA)") != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }
    if (validateWithRDFEventReader(treeNameB, expectedEventDataB, checkEntryB, "RDFEventReader(TTree TestingModelB)") != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }
    if (validateWithRDFEventReader(treeNameC, expectedEventDataC, checkEntryC, "RDFEventReader(TTree TestingModelC)") != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    // Run validations with multi-RDF RDFEventReader
    std::array<ROOT::RDF::RNode, 3> allRDF{
        ROOT::RDataFrame{ntupleNameA, fileName},
        ROOT::RDataFrame{ntupleNameB, fileName},
        ROOT::RDataFrame{ntupleNameC, fileName}
    };
    Mustard::Data::RDFEventReader<int, TestingModelA, TestingModelB, TestingModelC> multiReader{allRDF, "EvtID"};

    gtl::flat_hash_set<int> expectedEventIDSetA;
    gtl::flat_hash_set<int> expectedEventIDSetB;
    gtl::flat_hash_set<int> expectedEventIDSetC;
    const auto makeExpectedEventIDSet{[&](const auto& expectedEventData, auto& eventIDSet) {
        eventIDSet.reserve(expectedEventData.size());
        for (auto&& event : std::as_const(expectedEventData)) {
            eventIDSet.insert(Get<"EvtID">(*event[0]));
        }
    }};
    makeExpectedEventIDSet(expectedEventDataA, expectedEventIDSetA);
    makeExpectedEventIDSet(expectedEventDataB, expectedEventIDSetB);
    makeExpectedEventIDSet(expectedEventDataC, expectedEventIDSetC);

    const auto multiCount{multiReader.NEvent()};
    const auto multiData{multiReader.ReadNext(multiCount)};

    gtl::flat_hash_set<int> actualEventIDSetA;
    gtl::flat_hash_set<int> actualEventIDSetB;
    gtl::flat_hash_set<int> actualEventIDSetC;
    actualEventIDSetA.reserve(nEventA);
    actualEventIDSetB.reserve(nEventB);
    actualEventIDSetC.reserve(nEventC);
    for (auto&& [eventA, eventB, eventC] : std::as_const(multiData)) {
        if (eventA.empty() and eventB.empty() and eventC.empty()) {
            return fail("Multi-model RDFEventReader returned empty data for an event");
        }
        gtl::vector<int> eventIDABC;
        eventIDABC.reserve(3);
        const auto checkEvent{[&](const auto& event, auto& eventIDSet) {
            if (event.empty()) {
                return EXIT_SUCCESS;
            }
            if (std::ranges::any_of(event, [](const auto& entry) { return entry == nullptr; })) {
                return fail("Multi-model RDFEventReader returned null entry in an event");
            }
            if (not std::ranges::all_of(event, [&](const auto& entry) { return Get<"EvtID">(*entry) == Get<"EvtID">(*event[0]); })) {
                return fail("Multi-model RDFEventReader returned entries with mismatched EvtID in an event");
            }
            eventIDSet.insert(eventIDABC.emplace_back(Get<"EvtID">(*event[0])));
            return EXIT_SUCCESS;
        }};
        checkEvent(eventA, actualEventIDSetA);
        checkEvent(eventB, actualEventIDSetB);
        checkEvent(eventC, actualEventIDSetC);
        if (not std::ranges::all_of(eventIDABC, [&](int id) { return id == eventIDABC[0]; })) {
            return fail("Multi-model RDFEventReader returned entries with mismatched EvtID across models in an event");
        }
    }

    // Test Reset, ReadNext, SkipNext, and Exhaust for multi RDFEventReader
    const auto testMultiEventReaderReset{[&](std::array<std::string_view, 3> dataName,
                                             std::string_view sourceName) -> int {
        std::array<ROOT::RDF::RNode, 3> rdfs{
            ROOT::RDataFrame{dataName[0], fileName},
            ROOT::RDataFrame{dataName[1], fileName},
            ROOT::RDataFrame{dataName[2], fileName},
        };
        Mustard::Data::RDFEventReader<int, TestingModelA, TestingModelB, TestingModelC> reader{rdfs, "EvtID"};

        reader.Exhaust();
        if (not reader.Exhausted()) {
            return fail(fmt::format("{} not exhausted after Exhaust()", sourceName));
        }

        reader.Reset();

        const auto multiCount{reader.NEvent()};
        if (multiCount <= 0) {
            return fail(fmt::format("{} event count is {} after reset", sourceName, multiCount));
        }

        // Helper: validate cross-model EvtID alignment for a single event tuple and its sub-events.
        // Returns EXIT_SUCCESS when all non-empty sub-events share the same EvtID, and no null entry exists.
        const auto checkEvtIDAlignment{[&](const auto& eventTuple, std::string_view context) -> int {
            const auto& [eventA, eventB, eventC]{eventTuple};
            if (eventA.empty() and eventB.empty() and eventC.empty()) {
                return fail(fmt::format("{} {} returned empty events for all models", sourceName, context));
            }
            gtl::vector<int> evtIDs;
            evtIDs.reserve(3);
            for (const auto& event : {std::cref(eventA), std::cref(eventB), std::cref(eventC)}) {
                if (event.get().empty()) { continue; }
                if (event.get()[0] == nullptr) {
                    return fail(fmt::format("{} {} returned null entry", sourceName, context));
                }
                evtIDs.push_back(Get<"EvtID">(*event.get()[0]));
            }
            if (not evtIDs.empty() and not std::ranges::all_of(evtIDs, [&](int id) { return id == evtIDs[0]; })) {
                return fail(fmt::format("{} {} cross-model EvtID mismatch", sourceName, context));
            }
            return EXIT_SUCCESS;
        }};

        // ReadNext(): single event tuple
        if (checkEvtIDAlignment(reader.ReadNext(), "ReadNext()") != EXIT_SUCCESS) {
            return EXIT_FAILURE;
        }

        // ReadNext(2): batch of event tuples
        auto batch{reader.ReadNext(2)};
        if (batch.size() != 2) {
            return fail(fmt::format("{} ReadNext(2) size mismatch. expected=2, actual={}",
                                    sourceName, batch.size()));
        }
        for (gsl::index i{}; i < 2; ++i) {
            if (checkEvtIDAlignment(batch[i], fmt::format("ReadNext(2)[{}]", i)) != EXIT_SUCCESS) {
                return EXIT_FAILURE;
            }
        }

        // SkipNext(): skip 1 event
        reader.SkipNext();

        // SkipNext(2): skip 2 events
        reader.SkipNext(2);

        // Read one more event tuple (should be at event index 1 + 2 + 1 + 2 = 6)
        if (checkEvtIDAlignment(reader.ReadNext(), "ReadNext() after skip") != EXIT_SUCCESS) {
            return EXIT_FAILURE;
        }

        // Exhaust the rest
        reader.Exhaust();
        if (not reader.Exhausted()) {
            return fail(fmt::format("{} not exhausted after Exhaust()", sourceName));
        }

        return EXIT_SUCCESS;
    }};

    if (testMultiEventReaderReset({ntupleNameA, ntupleNameB, ntupleNameC}, "MultiRDFEventReader+Reset(RNTuple)") != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }
    if (testMultiEventReaderReset({treeNameA, treeNameB, treeNameC}, "MultiRDFEventReader+Reset(TTree)") != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
