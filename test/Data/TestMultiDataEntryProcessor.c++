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
#include "Mustard/Data/Object/Tuple.h++"
#include "Mustard/Data/Processing/Processor.h++"
#include "Mustard/Env/MPIEnv.h++"
#include "Mustard/IO/PrettyLog.h++"

#include "TestingMultiDataModel.h++"

#include "ROOT/RDataFrame.hxx"

#include "mplr/mplr.hpp"

#include "muc/array"
#include "muc/utility"

#include "fmt/format.h"

#include <array>
#include <cstdlib>
#include <functional>
#include <source_location>
#include <string_view>

auto main(int argc, char* argv[]) -> int {
    Mustard::CLI::BasicCLI<> cli;
    cli->add_argument("-n", "--n-entry-a").help("Number of entries for TestingModelA").default_value(1000000ull).required().nargs(1).scan<'i', unsigned long long>();
    cli->add_argument("-m", "--n-entry-b").help("Number of entries for TestingModelB").default_value(1500000ull).required().nargs(1).scan<'i', unsigned long long>();
    cli->add_argument("-p", "--n-entry-c").help("Number of entries for TestingModelC").default_value(800000ull).required().nargs(1).scan<'i', unsigned long long>();
    Mustard::Env::MPIEnv env{argc, argv, cli};

    const auto nEntryExpectedA{cli->get<unsigned long long>("--n-entry-a")};
    const auto nEntryExpectedB{cli->get<unsigned long long>("--n-entry-b")};
    const auto nEntryExpectedC{cli->get<unsigned long long>("--n-entry-c")};

    constexpr auto fileName{"test_multi_data_entry_io.root"};
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

    const auto worldComm{mplr::comm_world()};
    const auto validateWithProcessor{[&](std::array<std::string_view, 3> dataName, std::string_view sourceName) -> int {
        std::array<ROOT::RDF::RNode, 3> allRDF{
            ROOT::RDataFrame{dataName[0], fileName},
            ROOT::RDataFrame{dataName[1], fileName},
            ROOT::RDataFrame{dataName[2], fileName}
        };

        muc::arrayull<3> nEntryLocal{};
        const auto countEntries{[&](bool bypass,
                                    const Mustard::Data::ArcTuple<TestingModelA>& entryA,
                                    const Mustard::Data::ArcTuple<TestingModelB>& entryB,
                                    const Mustard::Data::ArcTuple<TestingModelC>& entryC) {
            if (bypass) {
                return;
            }
            if (entryA == nullptr and entryB == nullptr and entryC == nullptr) {
                Mustard::Throw<std::runtime_error>(fmt::format("{} encountered null entries for all models", sourceName));
            }
            if (entryA) {
                ++nEntryLocal[0];
            }
            if (entryB) {
                ++nEntryLocal[1];
            }
            if (entryC) {
                ++nEntryLocal[2];
            }
        }};

        Mustard::Data::Processor processor;
        processor.Run<TestingModelA, TestingModelB, TestingModelC>(allRDF, countEntries);

        muc::arrayull<3> nEntry{};
        worldComm.reduce(std::plus{}, 0, nEntryLocal.data(), nEntry.data(), mplr::contiguous_layout<unsigned long long>{3});
        if (worldComm.rank() == 0) {
            if (nEntry[0] != nEntryExpectedA) {
                return fail(fmt::format("{} TestingModelA entry count mismatch. expected={}, actual={}", sourceName, nEntryExpectedA, nEntry[0]));
            }
            if (nEntry[1] != nEntryExpectedB) {
                return fail(fmt::format("{} TestingModelB entry count mismatch. expected={}, actual={}", sourceName, nEntryExpectedB, nEntry[1]));
            }
            if (nEntry[2] != nEntryExpectedC) {
                return fail(fmt::format("{} TestingModelC entry count mismatch. expected={}, actual={}", sourceName, nEntryExpectedC, nEntry[2]));
            }
        }
        return EXIT_SUCCESS;
    }};

    if (validateWithProcessor({ntupleNameA, ntupleNameB, ntupleNameC}, "Processor(RNTuple)") != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }
    if (validateWithProcessor({treeNameA, treeNameB, treeNameC}, "Processor(TTree)") != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
