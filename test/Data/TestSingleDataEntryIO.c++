// Copyright (C) 2020-2025  Mustard developers
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
#include "Mustard/Data/Processing/RDFReader.h++"
#include "Mustard/Data/Processing/RNTupleWriter.h++"
#include "Mustard/Data/Processing/TTreeWriter.h++"
#include "Mustard/Env/BasicEnv.h++"
#include "Mustard/IO/File.h++"
#include "Mustard/IO/Print.h++"

#include "TestingSingleDataModel.h++"

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
    cli->add_argument("-n", "--n-entry").help("Number of entries to generate").default_value(1000000ull).required().nargs(1).scan<'i', unsigned long long>();
    Mustard::Env::BasicEnv env{argc, argv, cli};

    const auto nEntry{cli->get<unsigned long long>("--n-entry")};
    constexpr auto fileName{"test_single_data_entry_io.root"};
    constexpr auto ntupleName{"RNTuple"};
    constexpr auto treeName{"TTree"};

    const auto fail{[](std::string_view message, const std::source_location& location = std::source_location::current()) {
        Mustard::PrintError(message, location);
        return EXIT_FAILURE;
    }};

    Mustard::Data::ArcTupleVector<TestingModel> expected;
    for (gsl::index i{}; i < muc::to_signed(nEntry); ++i) {
        expected.emplace_back(MakeEntry(i, i));
    }

    {
        Mustard::File<TFile> outputFile{fileName, "RECREATE"};

        const auto fillWriter{[&]<typename TWriter>(TWriter& writer, std::string_view writerName) -> int {
            writer.Fill(expected[0]);

            auto arcEntry{Mustard::Data::MakeArcTuple<TestingModel>(*expected[1])};
            writer.Fill(std::move(arcEntry));
            writer.Flush();

            Mustard::Data::ArcTupleVector<TestingModel> batch;
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

        Mustard::Data::RNTupleWriter<TestingModel> ntupleWriter{ntupleName};
        if (fillWriter(ntupleWriter, "RNTupleWriter") != EXIT_SUCCESS) {
            return EXIT_FAILURE;
        }

        Mustard::Data::TTreeWriter<TestingModel> treeWriter{treeName};
        if (fillWriter(treeWriter, "TTreeWriter") != EXIT_SUCCESS) {
            return EXIT_FAILURE;
        }
    }

    const auto checkRow{[&](gsl::index i, const Mustard::Data::Tuple<TestingModel>& actual,
                            std::string_view sourceName) -> int {
        if (actual == *expected[i]) {
            return EXIT_SUCCESS;
        }
        Mustard::PrintError(fmt::format("{} row {} mismatch details:", sourceName, i));
        std::size_t mismatchCount{};
        const auto logMismatch{[&]<muc::ceta_string AName>() {
            if (Get<AName>(actual) != Get<AName>(*expected[i])) {
                ++mismatchCount;
                Mustard::PrintError(fmt::format("  - field '{}' mismatch", AName.sv()));
            }
        }};

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
        return fail(fmt::format("{} row content mismatch at index {}", sourceName, i));
    }};

    const auto validateWithRDFEntryReader{[&](std::string_view dataName, std::string_view sourceName) -> int {
        ROOT::RDataFrame rdf{dataName, fileName};
        Mustard::Data::RDFEntryReader<TestingModel> reader{rdf};
        const auto count{reader.NEntry()};
        if (count != ssize(expected)) {
            return fail(fmt::format("{} entry count mismatch. expected={}, actual={}", sourceName, expected.size(), count));
        }

        decltype(reader.Read(0, count)) data;
        constexpr auto readChunkSize{256};
        for (gsl::index offset{}; offset < count; offset += readChunkSize) {
            auto chunk{reader.Read(offset, std::min(offset + readChunkSize, count))};
            data.insert(data.end(),
                        std::make_move_iterator(chunk.begin()),
                        std::make_move_iterator(chunk.end()));
        }
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

    if (validateWithRDFEntryReader(ntupleName, "RDFEntryReader(RNTuple)") != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }
    if (validateWithRDFEntryReader(treeName, "RDFEntryReader(TTree)") != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
