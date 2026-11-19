// -*- C++ -*-
//
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
#include "Mustard/Env/MPIEnv.h++"
#include "Mustard/Execution/Executor.h++"
#include "Mustard/IO/PrettyLog.h++"
#include "Mustard/Testing/TestDispatcher/TestDispatcher.h++"

#include "mp1/mp1.hpp"

#include "gsl/gsl"

#include <algorithm>
#include <charconv>
#include <cstddef>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

namespace Mustard::Testing {

namespace {

namespace TestDispatcherSection {

constexpr auto nTask{20000};

// Open MPI 6.0.0 (including its release candidates) is known to hang in this
// test, so skip it for that version only; later 6.0.x releases run normally.
// Revisit once Open MPI 6.0.0 is no longer used in CI.
auto IsOpenMPI600() -> bool {
    const auto libraryVersion{mp1::get_library_version()};
    constexpr std::string_view vendor{"Open MPI v"};
    const auto versionBegin{libraryVersion.find(vendor)};
    if (versionBegin == std::string::npos) {
        return false;
    }
    const auto first{libraryVersion.data() + versionBegin + vendor.size()};
    const auto last{libraryVersion.data() + libraryVersion.size()};
    int major{};
    int minor{};
    int patch{};
    const auto [majorEnd, majorError]{std::from_chars(first, last, major)};
    if (majorError != std::errc{}) {
        return false;
    }
    if (majorEnd == last or *majorEnd != '.') {
        return false;
    }
    const auto [minorEnd, minorError]{std::from_chars(majorEnd + 1, last, minor)};
    if (minorError != std::errc{}) {
        return false;
    }
    if (minorEnd == last or *minorEnd != '.') {
        return false;
    }
    const auto [patchEnd, patchError]{std::from_chars(minorEnd + 1, last, patch)};
    (void)patchEnd;
    return patchError == std::errc{} and major == 6 and minor == 0 and patch == 0;
}

auto CheckTaskSet(int expectedN, const std::vector<int>& localTask) -> void {
    const auto& worldComm{mp1::comm_world()};
    const auto localSize{gsl::narrow<int>(localTask.size())};
    std::vector<int> size(worldComm.rank() == 0 ? worldComm.size() : 0);
    mp1::gather(worldComm, 0, localSize, size);

    std::vector<mp1::count_t> count;
    std::vector<mp1::count_t> disp;
    std::vector<int> allTask;
    if (worldComm.rank() == 0) {
        count.resize(worldComm.size());
        disp.resize(worldComm.size());
        for (int i{}; i < worldComm.size(); ++i) {
            count[i] = size[i];
            if (i > 0) {
                disp[i] = disp[i - 1] + size[i - 1];
            }
        }
        allTask.resize(disp.back() + count.back());
    }
    mp1::gatherv(worldComm, 0, localTask, allTask,
                 mp1::blocks{.counts = count, .displacements = disp});

    if (worldComm.rank() == 0) {
        if (allTask.size() != static_cast<std::size_t>(expectedN)) {
            Throw<std::runtime_error>("A dispatcher did not deliver every task exactly once.");
        }
        std::ranges::sort(allTask);
        for (int i{}; i < expectedN; ++i) {
            if (allTask[i] != i) {
                Throw<std::runtime_error>("A dispatcher delivered an invalid task set.");
            }
        }
    }
}

auto RunDispatcher(std::string_view code) -> void {
    Executor<int> executor{"Dispatcher test", "Dispatch", "task", code};
    std::vector<int> localTask;
    localTask.reserve(nTask);
    executor.Run(nTask, [&](int i) { localTask.push_back(i); });
    CheckTaskSet(nTask, localTask);
}

} // namespace TestDispatcherSection

} // namespace

TestDispatcher::TestDispatcher() :
    Subprogram{"TestDispatcher", "Test all Mustard dispatchers."} {}

auto TestDispatcher::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    Mustard::Env::MPIEnv env{argc, argv, cli};

    using namespace TestDispatcherSection;

    if (IsOpenMPI600()) {
        MasterPrintWarning("Skipping TestDispatcher under Open MPI 6.0.0.");
        return EXIT_SUCCESS;
    }

    const auto& worldComm{mp1::comm_world()};
    RunDispatcher("stat");
    RunDispatcher("mw");
    if (env.OnSingleNode()) {
        RunDispatcher("shm");
    }
    if (worldComm.size() == 1) {
        RunDispatcher("seq");
    }
    if (worldComm.size() >= 2) {
        // The cluster-aware dispatcher also runs on a single node and only needs more than one rank.
        RunDispatcher("cmw");
    }

    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
