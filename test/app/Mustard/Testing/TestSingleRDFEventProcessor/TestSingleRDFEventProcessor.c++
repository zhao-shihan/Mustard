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
#include "Mustard/Env/BasicEnv.h++"
#include "Mustard/IO/PrettyLog.h++"
#include "Mustard/Testing/SingleTestingDataModel.h++"
#include "Mustard/Testing/TestSingleRDFEventProcessor/TestSingleRDFEventProcessor.h++"

#include <algorithm>
#include <cstdlib>
#include <source_location>
#include <stdexcept>
#include <string_view>

namespace Mustard::Testing {

TestSingleRDFEventProcessor::TestSingleRDFEventProcessor() :
    Subprogram{"TestSingleRDFEventProcessor", "Test Mustard::Data::Processor::Run over events with single RDF."} {}

auto TestSingleRDFEventProcessor::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    cli->add_argument("-n", "--n-event").help("Number of events to generate").default_value(1000000ull).required().nargs(1).scan<'i', unsigned long long>();
    Mustard::Env::MPIEnv env{argc, argv, cli};

    const auto nEventExpected{cli->get<unsigned long long>("--n-event")};
    constexpr auto fileName{"test_single_data_event_io.root"};
    constexpr auto ntupleName{"RNTuple"};
    constexpr auto treeName{"TTree"};

    const auto fail{[](std::string_view message, const std::source_location& location = std::source_location::current()) {
        Mustard::PrintError(message, location);
        return EXIT_FAILURE;
    }};

    const auto worldComm{mplr::comm_world()};

    const auto validateWithProcessor{[&](std::string_view dataName, std::string_view sourceName) -> int {
        ROOT::RDataFrame rdf{dataName, fileName};

        gtl::vector<int> localEventIDList;
        const auto collectEventIDs{[&](bool bypass, const Mustard::Data::ArcTupleVector<TestingModel>& event) {
            if (bypass) {
                return;
            }
            if (event.empty()) {
                Mustard::Throw<std::runtime_error>(fmt::format("{} empty event found. source={}", sourceName, dataName));
            }
            const auto eventID{F<"EvtID">(*event[0])};
            if (not std::ranges::all_of(event, [&](const auto& e) { return F<"EvtID">(*e) == eventID; })) {
                Mustard::Throw<std::runtime_error>(fmt::format("{} event ID mismatch within the same event. eventID={}, source={}", sourceName, *eventID, dataName));
            }
            localEventIDList.push_back(eventID);
        }};

        Mustard::Data::Processor processor;
        processor.Run<TestingModel>(rdf, int{}, "EvtID", collectEventIDs);

        gtl::vector<int> eventIDList;
        if (worldComm.rank() == 0) {
            gtl::vector<int> size(worldComm.size());
            worldComm.gather(0, static_cast<int>(localEventIDList.size()), size.data());

            mplr::displacements disp(worldComm.size());
            disp[0] = 0;
            for (int i = 1; i < worldComm.size(); ++i) {
                disp[i] = disp[i - 1] + size[i - 1];
            }
            mplr::contiguous_layouts<int> layout(worldComm.size());
            std::ranges::transform(size, layout.begin(), [](auto n) { return mplr::contiguous_layout<int>(n); });

            eventIDList.resize(disp[worldComm.size() - 1] + size.back());
            worldComm.gatherv(0, localEventIDList.data(), mplr::contiguous_layout<int>{localEventIDList.size()},
                              eventIDList.data(), layout, disp);
        } else {
            worldComm.gather(0, static_cast<int>(localEventIDList.size()));
            worldComm.gatherv(0, localEventIDList.data(), mplr::contiguous_layout<int>{localEventIDList.size()});
        }

        if (worldComm.rank() == 0) {
            if (eventIDList.size() != nEventExpected) {
                return fail(fmt::format("{} event count mismatch. expected={}, actual={}", sourceName, nEventExpected, eventIDList.size()));
            }

            gtl::flat_hash_set<int> eventIDSet;
            eventIDSet.reserve(eventIDList.size());
            for (auto&& id : std::as_const(eventIDList)) {
                const auto [_, inserted]{eventIDSet.insert(id)};
                if (not inserted) {
                    return fail(fmt::format("{} duplicate event ID found: {}. source={}", sourceName, id, dataName));
                }
            }

            if (eventIDSet.size() != eventIDList.size()) {
                return fail(fmt::format("{} unique event count mismatch (duplicates found). expected={}, actual={}", sourceName, eventIDList.size(), eventIDSet.size()));
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

} // namespace Mustard::Testing
