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
#include "Mustard/Data/Object/Tuple.h++"
#include "Mustard/Data/Processing/Processor.h++"
#include "Mustard/Env/MPIEnv.h++"
#include "Mustard/IO/PrettyLog.h++"

#include "TestingSingleDataModel.h++"

#include "ROOT/RDataFrame.hxx"

#include "mplr/mplr.hpp"

#include "muc/utility"

#include "fmt/format.h"

#include <cstdlib>
#include <functional>
#include <source_location>
#include <stdexcept>
#include <string_view>

auto main(int argc, char* argv[]) -> int {
    Mustard::CLI::BasicCLI<> cli;
    cli->add_argument("-n", "--n-entry").help("Number of entries to generate").default_value(1000000ull).required().nargs(1).scan<'i', unsigned long long>();
    Mustard::Env::MPIEnv env{argc, argv, cli};

    const auto nEntryExpected{cli->get<unsigned long long>("--n-entry")};
    constexpr auto fileName{"test_single_data_entry_io.root"};
    constexpr auto ntupleName{"RNTuple"};
    constexpr auto treeName{"TTree"};

    const auto fail{[](std::string_view message, const std::source_location& location = std::source_location::current()) {
        Mustard::PrintError(message, location);
        return EXIT_FAILURE;
    }};

    const auto worldComm{mplr::comm_world()};
    const auto validateWithProcessor{[&](std::string_view dataName, std::string_view sourceName) -> int {
        ROOT::RDataFrame rdf{dataName, fileName};

        unsigned long long nEntryLocal{};
        const auto countEntries{[&](bool bypass, Mustard::Data::ArcTuple<TestingModel> entry) {
            if (bypass) {
                return;
            }
            if (entry == nullptr) {
                Mustard::Throw<std::runtime_error>(fmt::format("{} encountered null entry", sourceName));
            }
            ++nEntryLocal;
        }};

        Mustard::Data::Processor processor;
        processor.Run<TestingModel>(rdf, countEntries);

        unsigned long long nEntry{};
        worldComm.reduce(std::plus{}, 0, nEntryLocal, nEntry);
        if (worldComm.rank() == 0) {
            if (nEntry != nEntryExpected) {
                return fail(fmt::format("{} entry count mismatch. expected={}, actual={}", sourceName, nEntryExpected, nEntry));
            }
        }
        return EXIT_SUCCESS;
    }};

    if (validateWithProcessor(ntupleName, "Processor(RNTuple)") != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }
    if (validateWithProcessor(treeName, "Processor(TTree)") != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
