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

#include "Mustard/CLI/BasicCLI.h++"
#include "Mustard/Data/Container/ArcTupleVector.h++"
#include "Mustard/Data/Model.h++"
#include "Mustard/Data/Object/Tuple.h++"
#include "Mustard/Data/Processing/RDFEntryReader.h++"
#include "Mustard/Data/Processing/RNTupleWriter.h++"
#include "Mustard/Data/Processing/TTreeWriter.h++"
#include "Mustard/Env/BasicEnv.h++"
#include "Mustard/IO/File.h++"
#include "Mustard/IO/Print.h++"

#include "TestingMultiDataModel.h++"

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
    Mustard::CLI::BasicCLI<> cli;
    cli->add_argument("-n", "--n-entry-a").help("Number of entries for TestingModelA").default_value(1000000ull).required().nargs(1).scan<'i', unsigned long long>();
    cli->add_argument("-m", "--n-entry-b").help("Number of entries for TestingModelB").default_value(1500000ull).required().nargs(1).scan<'i', unsigned long long>();
    cli->add_argument("-p", "--n-entry-c").help("Number of entries for TestingModelC").default_value(800000ull).required().nargs(1).scan<'i', unsigned long long>();
    Mustard::Env::BasicEnv env{argc, argv, cli};

    const auto nEntryA{cli->get<unsigned long long>("--n-entry-a")};
    const auto nEntryB{cli->get<unsigned long long>("--n-entry-b")};
    const auto nEntryC{cli->get<unsigned long long>("--n-entry-c")};
    const auto fileName{"test_multi_data_entry_io.root"};
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
    Mustard::Data::ArcTupleVector<TestingModelA> expectedA;
    for (gsl::index i{}; i < muc::to_signed(nEntryA); ++i) {
        expectedA.emplace_back(MakeEntryA(i, i));
    }
    Mustard::Data::ArcTupleVector<TestingModelB> expectedB;
    for (gsl::index i{}; i < muc::to_signed(nEntryB); ++i) {
        expectedB.emplace_back(MakeEntryB(i, i));
    }
    Mustard::Data::ArcTupleVector<TestingModelC> expectedC;
    for (gsl::index i{}; i < muc::to_signed(nEntryC); ++i) {
        expectedC.emplace_back(MakeEntryC(i, i));
    }

    // Write data using RNTupleWriter and TTreeWriter
    {
        Mustard::File<TFile> outputFile{fileName, "RECREATE"};

        const auto fillWriter{[&]<typename AWriter, typename Model>(
                                  AWriter& writer, const Mustard::Data::ArcTupleVector<Model>& expected,
                                  std::string_view writerName) -> int {
            if (expected.empty()) {
                return EXIT_SUCCESS;
            }
            writer.Fill(expected[0]);

            auto arcEntry{Mustard::Data::MakeArcTuple<Model>(*expected[1])};
            writer.Fill(std::move(arcEntry));
            writer.Flush();

            Mustard::Data::ArcTupleVector<Model> batch;
            batch.reserve(expected.size() - 2);
            for (gsl::index i{2}; i < ssize(expected); ++i) {
                batch.emplace_back(expected[i]);
            }
            writer.Fill(std::move(batch));

            if (writer.NEntry() != ssize(expected)) {
                return fail(fmt::format("{} entry count mismatch after Fill/Flush. expected={}, actual={}",
                                        writerName, expected.size(), writer.NEntry()));
            }
            return EXIT_SUCCESS;
        }};

        Mustard::Data::RNTupleWriter<TestingModelA> ntupleWriterA{ntupleNameA};
        if (fillWriter(ntupleWriterA, expectedA, "RNTupleWriter(TestingModelA)") != EXIT_SUCCESS) {
            return EXIT_FAILURE;
        }

        Mustard::Data::RNTupleWriter<TestingModelB> ntupleWriterB{ntupleNameB};
        if (fillWriter(ntupleWriterB, expectedB, "RNTupleWriter(TestingModelB)") != EXIT_SUCCESS) {
            return EXIT_FAILURE;
        }

        Mustard::Data::RNTupleWriter<TestingModelC> ntupleWriterC{ntupleNameC};
        if (fillWriter(ntupleWriterC, expectedC, "RNTupleWriter(TestingModelC)") != EXIT_SUCCESS) {
            return EXIT_FAILURE;
        }

        Mustard::Data::TTreeWriter<TestingModelA> treeWriterA{treeNameA};
        if (fillWriter(treeWriterA, expectedA, "TTreeWriter(TestingModelA)") != EXIT_SUCCESS) {
            return EXIT_FAILURE;
        }

        Mustard::Data::TTreeWriter<TestingModelB> treeWriterB{treeNameB};
        if (fillWriter(treeWriterB, expectedB, "TTreeWriter(TestingModelB)") != EXIT_SUCCESS) {
            return EXIT_FAILURE;
        }

        Mustard::Data::TTreeWriter<TestingModelC> treeWriterC{treeNameC};
        if (fillWriter(treeWriterC, expectedC, "TTreeWriter(TestingModelC)") != EXIT_SUCCESS) {
            return EXIT_FAILURE;
        }
    }

    // Verification functions
    const auto checkRowA{[&](gsl::index i, const Mustard::Data::Tuple<TestingModelA>& actual,
                             std::string_view sourceName) -> int {
        if (actual == *expectedA[i]) {
            return EXIT_SUCCESS;
        }
        Mustard::PrintError(fmt::format("{} row {} mismatch details:", sourceName, i));
        std::size_t mismatchCount{};
        const auto logMismatch{[&]<muc::ceta_string AName>() {
            if (Get<AName>(actual) != Get<AName>(*expectedA[i])) {
                ++mismatchCount;
                Mustard::PrintError(fmt::format("  - field '{}' mismatch", AName.sv()));
            }
        }};
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
        return fail(fmt::format("{} row content mismatch at index {}", sourceName, i));
    }};

    const auto checkRowB{[&](gsl::index i, const Mustard::Data::Tuple<TestingModelB>& actual,
                             std::string_view sourceName) -> int {
        if (actual == *expectedB[i]) {
            return EXIT_SUCCESS;
        }
        Mustard::PrintError(fmt::format("{} row {} mismatch details:", sourceName, i));
        std::size_t mismatchCount{};
        const auto logMismatch{[&]<muc::ceta_string AName>() {
            if (Get<AName>(actual) != Get<AName>(*expectedB[i])) {
                ++mismatchCount;
                Mustard::PrintError(fmt::format("  - field '{}' mismatch", AName.sv()));
            }
        }};
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
        return fail(fmt::format("{} row content mismatch at index {}", sourceName, i));
    }};

    const auto checkRowC{[&](gsl::index i, const Mustard::Data::Tuple<TestingModelC>& actual,
                             std::string_view sourceName) -> int {
        if (actual == *expectedC[i]) {
            return EXIT_SUCCESS;
        }
        Mustard::PrintError(fmt::format("{} row {} mismatch details:", sourceName, i));
        std::size_t mismatchCount{};
        const auto logMismatch{[&]<muc::ceta_string AName>() {
            if (Get<AName>(actual) != Get<AName>(*expectedC[i])) {
                ++mismatchCount;
                Mustard::PrintError(fmt::format("  - field '{}' mismatch", AName.sv()));
            }
        }};
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
        return fail(fmt::format("{} row content mismatch at index {}", sourceName, i));
    }};

    // Validate with single RDFEntryReader (optional)
    const auto validateSingleRDFEntryReader{[&]<typename Model>(std::string_view dataName, const Mustard::Data::ArcTupleVector<Model>& expected,
                                                                auto checkRow, std::string_view sourceName) -> int {
        ROOT::RDataFrame rdf{dataName, fileName};
        Mustard::Data::RDFEntryReader<Model> reader{rdf};
        const auto count{reader.NEntry()};
        if (count != ssize(expected)) {
            return fail(fmt::format("{} entry count mismatch. expected={}, actual={}", sourceName, expected.size(), count));
        }
        const auto data{reader.ReadNext(count)};
        if (data.size() != expected.size()) {
            return fail(fmt::format("{} read size mismatch. expected={}, actual={}", sourceName, expected.size(), data.size()));
        }
        for (gsl::index i{}; i < ssize(data); ++i) {
            if (data[i] == nullptr) {
                return fail(fmt::format("{} returned null entry at index {}", sourceName, i));
            }
            if (checkRow(i, *data[i], sourceName) != EXIT_SUCCESS) {
                return EXIT_FAILURE;
            }
        }
        return EXIT_SUCCESS;
    }};

    // Validate with multi RDFEntryReader
    const auto validateMultiRDFEntryReader{[&](std::array<std::string_view, 3> dataName,
                                               std::string_view sourceName) -> int {
        std::array<ROOT::RDF::RNode, 3> rdfs{
            ROOT::RDataFrame{dataName[0], fileName},
            ROOT::RDataFrame{dataName[1], fileName},
            ROOT::RDataFrame{dataName[2], fileName},
        };
        Mustard::Data::RDFEntryReader<TestingModelA, TestingModelB, TestingModelC> reader{rdfs};
        const auto count{reader.NEntry()};
        // The total entry count should be the maximum of the three RDF entry counts
        const auto expectedCount{std::max({nEntryA, nEntryB, nEntryC})};
        if (count != static_cast<gsl::index>(expectedCount)) {
            return fail(fmt::format("{} entry count mismatch. expected={}, actual={}", sourceName, expectedCount, count));
        }

        const auto data{reader.ReadNext(count)};
        if (data.size() != static_cast<std::size_t>(count)) {
            return fail(fmt::format("{} read size mismatch. expected={}, actual={}", sourceName, count, data.size()));
        }

        // Check each entry
        for (gsl::index i{}; i < count; ++i) {
            const auto& tuple{data[i]};
            // For each model, if i < its size, compare; else the entry should be null? Actually, the tuple will contain std::optional?
            // According to RDFEntryReader multi implementation, each slot may hold a nullptr if the sub-reader has no data at that index.
            // We'll need to examine the tuple elements.
            if (i < ssize(expectedA)) {
                if (std::get<0>(tuple) == nullptr) {
                    return fail(fmt::format("{} TestingModelA entry {} is null but expected data", sourceName, i));
                }
                if (checkRowA(i, *std::get<0>(tuple), sourceName) != EXIT_SUCCESS) {
                    return EXIT_FAILURE;
                }
            } else {
                if (std::get<0>(tuple) != nullptr) {
                    return fail(fmt::format("{} TestingModelA entry {} is not null but expected null", sourceName, i));
                }
            }
            if (i < ssize(expectedB)) {
                if (std::get<1>(tuple) == nullptr) {
                    return fail(fmt::format("{} TestingModelB entry {} is null but expected data", sourceName, i));
                }
                if (checkRowB(i, *std::get<1>(tuple), sourceName) != EXIT_SUCCESS) {
                    return EXIT_FAILURE;
                }
            } else {
                if (std::get<1>(tuple) != nullptr) {
                    return fail(fmt::format("{} TestingModelB entry {} is not null but expected null", sourceName, i));
                }
            }
            if (i < ssize(expectedC)) {
                if (std::get<2>(tuple) == nullptr) {
                    return fail(fmt::format("{} TestingModelC entry {} is null but expected data", sourceName, i));
                }
                if (checkRowC(i, *std::get<2>(tuple), sourceName) != EXIT_SUCCESS) {
                    return EXIT_FAILURE;
                }
            } else {
                if (std::get<2>(tuple) != nullptr) {
                    return fail(fmt::format("{} TestingModelC entry {} is not null but expected null", sourceName, i));
                }
            }
        }
        return EXIT_SUCCESS;
    }};

    // Run validations
    // Single RDFEntryReader tests
    if (validateSingleRDFEntryReader(ntupleNameA, expectedA, checkRowA, "RDFEntryReader(RNTuple TestingModelA)") != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }
    if (validateSingleRDFEntryReader(ntupleNameB, expectedB, checkRowB, "RDFEntryReader(RNTuple TestingModelB)") != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }
    if (validateSingleRDFEntryReader(ntupleNameC, expectedC, checkRowC, "RDFEntryReader(RNTuple TestingModelC)") != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }
    if (validateSingleRDFEntryReader(treeNameA, expectedA, checkRowA, "RDFEntryReader(TTree TestingModelA)") != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }
    if (validateSingleRDFEntryReader(treeNameB, expectedB, checkRowB, "RDFEntryReader(TTree TestingModelB)") != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }
    if (validateSingleRDFEntryReader(treeNameC, expectedC, checkRowC, "RDFEntryReader(TTree TestingModelC)") != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    // Multi RDFEntryReader tests
    if (validateMultiRDFEntryReader({ntupleNameA, ntupleNameB, ntupleNameC}, "MultiRDFEntryReader(RNTuple)") != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }
    if (validateMultiRDFEntryReader({treeNameA, treeNameB, treeNameC}, "MultiRDFEntryReader(TTree)") != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    // Test Reset, ReadNext, SkipNext, and Exhaust for multi RDFEntryReader
    const auto testMultiReaderReset{[&](std::array<std::string_view, 3> dataName,
                                        std::string_view sourceName) -> int {
        std::array<ROOT::RDF::RNode, 3> rdfs{
            ROOT::RDataFrame{dataName[0], fileName},
            ROOT::RDataFrame{dataName[1], fileName},
            ROOT::RDataFrame{dataName[2], fileName},
        };
        Mustard::Data::RDFEntryReader<TestingModelA, TestingModelB, TestingModelC> reader{rdfs};

        reader.Exhaust();
        if (not reader.Exhausted()) {
            return fail(fmt::format("{} not exhausted after Exhaust()", sourceName));
        }

        reader.Reset();

        const auto count{reader.NEntry()};
        const auto expectedCount{std::max({nEntryA, nEntryB, nEntryC})};
        if (count != static_cast<gsl::index>(expectedCount)) {
            return fail(fmt::format("{} entry count mismatch after reset. expected={}, actual={}",
                                    sourceName, expectedCount, count));
        }

        // Helper: validate all 3 models in a single tuple at a given row index
        const auto checkAllModelsInTuple{[&](const auto& tuple, gsl::index rowIdx) -> int {
            if (auto& slot = std::get<0>(tuple); slot == nullptr) {
                return fail(fmt::format("{} ModelA is null at index {}", sourceName, rowIdx));
            } else if (checkRowA(rowIdx, *slot, sourceName) != EXIT_SUCCESS) {
                return EXIT_FAILURE;
            }
            if (auto& slot = std::get<1>(tuple); slot == nullptr) {
                return fail(fmt::format("{} ModelB is null at index {}", sourceName, rowIdx));
            } else if (checkRowB(rowIdx, *slot, sourceName) != EXIT_SUCCESS) {
                return EXIT_FAILURE;
            }
            if (auto& slot = std::get<2>(tuple); slot == nullptr) {
                return fail(fmt::format("{} ModelC is null at index {}", sourceName, rowIdx));
            } else if (checkRowC(rowIdx, *slot, sourceName) != EXIT_SUCCESS) {
                return EXIT_FAILURE;
            }
            return EXIT_SUCCESS;
        }};

        // ReadNext(): single tuple entry
        if (checkAllModelsInTuple(reader.ReadNext(), 0) != EXIT_SUCCESS) {
            return EXIT_FAILURE;
        }

        // ReadNext(3): batch
        auto batch{reader.ReadNext(3)};
        if (batch.size() != 3) {
            return fail(fmt::format("{} ReadNext(3) size mismatch. expected=3, actual={}",
                                    sourceName, batch.size()));
        }
        for (gsl::index i{}; i < 3; ++i) {
            if (checkAllModelsInTuple(batch[i], 1 + i) != EXIT_SUCCESS) {
                return EXIT_FAILURE;
            }
        }

        // SkipNext(): skip 1 entry
        reader.SkipNext();

        // SkipNext(5): skip 5 entries
        reader.SkipNext(5);

        // Read one more (should be at index 1 + 3 + 1 + 5 = 10)
        if (checkAllModelsInTuple(reader.ReadNext(), 10) != EXIT_SUCCESS) {
            return EXIT_FAILURE;
        }

        // Exhaust the rest
        reader.Exhaust();
        if (not reader.Exhausted()) {
            return fail(fmt::format("{} not exhausted after Exhaust()", sourceName));
        }

        return EXIT_SUCCESS;
    }};

    if (testMultiReaderReset({ntupleNameA, ntupleNameB, ntupleNameC}, "MultiRDFEntryReader+Reset(RNTuple)") != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }
    if (testMultiReaderReset({treeNameA, treeNameB, treeNameC}, "MultiRDFEntryReader+Reset(TTree)") != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
