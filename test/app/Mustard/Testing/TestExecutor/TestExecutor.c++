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
#include "Mustard/IO/Print.h++"
#include "Mustard/Testing/TestExecutor/TestExecutor.h++"

#include "mp1/mp1.hpp"

#include "muc/algorithm"
#include "muc/numeric"
#include "muc/utility"

#include "gsl/gsl"

#include <algorithm>
#include <string>
#include <thread>
#include <vector>

namespace Mustard::Testing {

using namespace std::chrono_literals;

TestExecutor::TestExecutor() :
    Subprogram{"TestExecutor", "Test Mustard::Executor."} {}

namespace {

namespace TestExecutorSection {

auto CheckIndexList(int truthN, const std::vector<int>& localIndexList) -> void {
    const auto& worldComm{mp1::comm_world()};
    const auto localSize{gsl::narrow<int>(localIndexList.size())};
    std::vector<int> size(worldComm.rank() == 0 ? worldComm.size() : 0);
    mp1::gather(worldComm, 0, localSize, size);

    std::vector<mp1::count_t> count;
    std::vector<mp1::count_t> disp;
    std::vector<int> indexList;
    if (worldComm.rank() == 0) {
        const auto n{muc::ranges::reduce(size, 0ll)};
        count.resize(worldComm.size());
        disp.resize(worldComm.size());
        for (int i{}; i < worldComm.size(); ++i) {
            count[i] = size[i];
            if (i > 0) {
                disp[i] = disp[i - 1] + size[i - 1];
            }
        }
        indexList.resize(n);
    }
    mp1::gatherv(worldComm, 0, localIndexList, indexList,
                 mp1::blocks{.counts = count, .displacements = disp});

    if (worldComm.rank() == 0) {
        muc::timsort(indexList);
        std::vector<int> diffList(indexList.size());
        muc::ranges::adjacent_difference(indexList, diffList.begin());
        const auto n{indexList.size()};
        const auto sum{muc::ranges::reduce(indexList, 0ll)};

        if (n != static_cast<std::size_t>(truthN)) {
            PrintError("n != truthN");
        }
        if (not std::ranges::all_of(diffList.cbegin() + 1, diffList.cend(), [](auto d) { return d == 1; })) {
            PrintError("not std::ranges::all_of(diffList.cbegin() + 1, diffList.cend(), [](auto d) { return d == 1; })");
        }
        if (sum != static_cast<long long>(n) * (static_cast<long long>(n) - 1) / 2) {
            PrintError("sum != n * (n - 1) / 2");
        }
    }
}

} // namespace TestExecutorSection

} // namespace

auto TestExecutor::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    cli->add_argument("n").help("Number of tasks.").default_value(10'000'000).required().nargs(1).scan<'i', int>();
    Mustard::Env::MPIEnv env{argc, argv, cli};

    using namespace TestExecutorSection;

    const auto n{cli->get<int>("n")};

    Executor<int> executor;

    const auto& worldComm{mp1::comm_world()};

    std::vector<int> localIndexList;
    executor.PrintProgress(false);
    executor.Run(n, [&](auto i) { localIndexList.emplace_back(i); });
    executor.PrintExecutionSummary();
    CheckIndexList(n, localIndexList);
    MasterPrintLn("");

    const auto originalExecutionName{executor.ExecutionName()};
    const auto originalOperationName{executor.OperationName()};
    const auto originalTaskName{executor.TaskName()};
    int namesCorrect{1};
    executor.Run("Temporary execution", "Temporary operation", "Temporary task", executor.NProcess(),
                 [&](auto) {
                     namesCorrect = namesCorrect and
                                    executor.ExecutionName() == "Temporary execution" and
                                    executor.OperationName() == "Temporary operation" and
                                    executor.TaskName() == "Temporary task";
                 });
    mp1::allreduce(worldComm, mp1::min{}, mp1::in_place, namesCorrect);
    if (namesCorrect == 0) {
        PrintError("Executor::Run did not apply the temporary names.");
    }
    if (executor.ExecutionName() != originalExecutionName or
        executor.OperationName() != originalOperationName or
        executor.TaskName() != originalTaskName) {
        PrintError("Executor::Run did not restore the original names.");
    }

    std::this_thread::sleep_for(1s);

    const auto bigN{std::min<long long>(1000ll * n, std::numeric_limits<int>::max() / 2)};
    localIndexList.clear();
    executor.PrintProgress(true);
    executor.Run(bigN, [&](auto i) { localIndexList.emplace_back(i); });
    executor.PrintExecutionSummary();
    CheckIndexList(bigN, localIndexList);
    MasterPrintLn("");

    std::this_thread::sleep_for(1s);

    localIndexList.clear();
    executor.Run(n, [&](auto i) {
        localIndexList.emplace_back(i);
        muc::cpu_relax();
    });
    executor.PrintExecutionSummary();
    CheckIndexList(n, localIndexList);
    MasterPrintLn("");

    std::this_thread::sleep_for(1s);

    executor.PrintProgressInterval(100ms);
    localIndexList.clear();
    executor.Run(n, [&](auto i) {
        localIndexList.emplace_back(i);
        muc::cpu_relax();
    });
    executor.PrintExecutionSummary();
    CheckIndexList(n, localIndexList);

    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
