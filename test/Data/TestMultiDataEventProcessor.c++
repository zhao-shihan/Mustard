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

#include "Mustard/CLI/MonteCarloCLI.h++"
#include "Mustard/Data/Container/ArcTupleVector.h++"
#include "Mustard/Data/Object/Tuple.h++"
#include "Mustard/Data/Processing/Processor.h++"
#include "Mustard/Env/MPIMonteCarloEnv.h++"
#include "Mustard/IO/PrettyLog.h++"

#include "TestingMultiDataModel.h++"

#include "ROOT/RDataFrame.hxx"

#include "gtl/phmap.hpp"
#include "gtl/vector.hpp"

#include "mplr/mplr.hpp"

#include "muc/utility"

#include "fmt/format.h"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <source_location>
#include <stdexcept>
#include <string_view>

auto main(int argc, char* argv[]) -> int {
    Mustard::CLI::MonteCarloCLI<> cli;
    cli->add_argument("-a", "--n-event-a").help("Number of events for TestingModelA").default_value(1000000ull).required().nargs(1).scan<'i', unsigned long long>();
    cli->add_argument("-b", "--n-event-b").help("Number of events for TestingModelB").default_value(1500000ull).required().nargs(1).scan<'i', unsigned long long>();
    cli->add_argument("-c", "--n-event-c").help("Number of events for TestingModelC").default_value(800000ull).required().nargs(1).scan<'i', unsigned long long>();
    Mustard::Env::MPIMonteCarloEnv<256> env{argc, argv, cli};

    const auto nEventA{cli->get<unsigned long long>("--n-event-a")};
    const auto nEventB{cli->get<unsigned long long>("--n-event-b")};
    const auto nEventC{cli->get<unsigned long long>("--n-event-c")};

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

    const auto worldComm{mplr::comm_world()};
    const auto validateWithProcessor{[&](std::array<std::string_view, 3> dataName, std::string_view sourceName) -> int {
        std::array<ROOT::RDF::RNode, 3> allRDF{
            ROOT::RDataFrame{dataName[0], fileName},
            ROOT::RDataFrame{dataName[1], fileName},
            ROOT::RDataFrame{dataName[2], fileName}
        };

        gtl::vector<int> localEventIDListA;
        gtl::vector<int> localEventIDListB;
        gtl::vector<int> localEventIDListC;
        const auto collectEventIDs{[&](bool bypass,
                                       const Mustard::Data::ArcTupleVector<TestingModelA>& eventA,
                                       const Mustard::Data::ArcTupleVector<TestingModelB>& eventB,
                                       const Mustard::Data::ArcTupleVector<TestingModelC>& eventC) {
            if (bypass) {
                return;
            }
            // Check 1: at least one event vector is non-empty
            if (eventA.empty() and eventB.empty() and eventC.empty()) {
                Mustard::Throw<std::runtime_error>(fmt::format("{} all event vectors are empty", sourceName));
            }
            // Check 2: all ArcTuples are non-null
            const auto checkNullEntries{[&](const auto& event, std::string_view modelName) {
                for (const auto& entry : event) {
                    if (entry == nullptr) {
                        Mustard::Throw<std::runtime_error>(fmt::format("{} null entry in {}", sourceName, modelName));
                    }
                }
            }};
            checkNullEntries(eventA, "TestingModelA");
            checkNullEntries(eventB, "TestingModelB");
            checkNullEntries(eventC, "TestingModelC");
            // Check 3: all EvtIDs are the same
            auto eventID{-1};
            const auto checkSingleEvent{[&](const auto& event) {
                for (auto&& entry : std::as_const(event)) {
                    const auto thisEventID{Get<"EvtID">(*entry)};
                    if (eventID == -1) {
                        eventID = thisEventID;
                        continue;
                    }
                    if (thisEventID != eventID) {
                        Mustard::Throw<std::runtime_error>(fmt::format("{} event ID mismatch within the same event. eventID={}, thisEventID={}", sourceName, eventID, *thisEventID));
                    }
                }
            }};
            checkSingleEvent(eventA);
            checkSingleEvent(eventB);
            checkSingleEvent(eventC);
            // Collect event IDs
            if (not eventA.empty()) {
                localEventIDListA.push_back(eventID);
            }
            if (not eventB.empty()) {
                localEventIDListB.push_back(eventID);
            }
            if (not eventC.empty()) {
                localEventIDListC.push_back(eventID);
            }
        }};

        Mustard::Data::Processor processor;
        processor.Run<TestingModelA, TestingModelB, TestingModelC>(allRDF, int{}, "EvtID", collectEventIDs);

        const auto gatherEventIDs{[&](const gtl::vector<int>& localList, gtl::vector<int>& gatheredList) {
            if (worldComm.rank() == 0) {
                gtl::vector<int> size(worldComm.size());
                worldComm.gather<int>(0, static_cast<int>(localList.size()), size.data());
                mplr::displacements disp(worldComm.size());
                for (int i = 1; i < worldComm.size(); ++i) {
                    disp[i] = disp[i - 1] + size[i - 1];
                }
                mplr::contiguous_layouts<int> layout(worldComm.size());
                std::ranges::transform(size, layout.begin(), [](auto n) { return mplr::contiguous_layout<int>(n); });
                gatheredList.resize(disp[worldComm.size() - 1] + size.back());
                worldComm.gatherv(0, localList.data(), mplr::contiguous_layout<int>{localList.size()}, gatheredList.data(), layout, disp);
            } else {
                worldComm.gather<int>(0, static_cast<int>(localList.size()));
                worldComm.gatherv(0, localList.data(), mplr::contiguous_layout<int>{localList.size()});
            }
        }};

        gtl::vector<int> eventIDListA;
        gtl::vector<int> eventIDListB;
        gtl::vector<int> eventIDListC;
        gatherEventIDs(localEventIDListA, eventIDListA);
        gatherEventIDs(localEventIDListB, eventIDListB);
        gatherEventIDs(localEventIDListC, eventIDListC);

        if (worldComm.rank() == 0) {
            const auto checkEventIDs{[&](const gtl::vector<int>& eventIDList, unsigned long long nEventExpected, std::string_view modelName) -> int {
                gtl::flat_hash_set<int> eventIDSet;
                eventIDSet.reserve(eventIDList.size());
                for (auto&& id : std::as_const(eventIDList)) {
                    const auto [_, inserted]{eventIDSet.insert(id)};
                    if (not inserted) {
                        return fail(fmt::format("{} duplicate event ID found in {}: {}", sourceName, modelName, id));
                    }
                }
                if (eventIDSet.size() != nEventExpected) {
                    return fail(fmt::format("{} {} event count mismatch. expected={}, actual={}", sourceName, modelName, nEventExpected, eventIDSet.size()));
                }
                return EXIT_SUCCESS;
            }};
            if (checkEventIDs(eventIDListA, nEventA, "TestingModelA") != EXIT_SUCCESS) {
                return EXIT_FAILURE;
            }
            if (checkEventIDs(eventIDListB, nEventB, "TestingModelB") != EXIT_SUCCESS) {
                return EXIT_FAILURE;
            }
            if (checkEventIDs(eventIDListC, nEventC, "TestingModelC") != EXIT_SUCCESS) {
                return EXIT_FAILURE;
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
