// Copyright (C) 2020-2025  The Mustard development team
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

#include "Mustard/Env/MPIEnv.h++"
#include "Mustard/Execution/Executor.h++"
#include "Mustard/IO/Print.h++"

#include "mplr/mplr.hpp"

#include "muc/algorithm"
#include "muc/numeric"

#include "gsl/gsl"

#include <algorithm>
#include <string>
#include <thread>

using namespace Mustard;
using namespace std::chrono_literals;

auto CheckIndexList(int truthN, const std::vector<int>& localIndexList) -> void {
    const auto worldComm{mplr::comm_world()};
    if (worldComm.rank() == 0) {
        std::vector<int> size(worldComm.size());
        worldComm.gather<int>(0, localIndexList.size(), size.data());
        const auto n{muc::ranges::reduce(size, 0ll)};

        mplr::displacements disp(worldComm.size());
        for (auto i{1}; i < worldComm.size(); ++i) {
            disp[i] = disp[i - 1] + size[i - 1];
        }
        mplr::contiguous_layouts<int> layout(worldComm.size());
        std::ranges::transform(size, layout.begin(), [](auto n) { return mplr::contiguous_layout<int>(n); });
        std::vector<int> indexList(n);
        worldComm.gatherv(0, localIndexList.data(), mplr::contiguous_layout<int>{localIndexList.size()},
                          indexList.data(), layout, disp);

        muc::timsort(indexList);
        std::vector<int> diffList(indexList.size());
        muc::ranges::adjacent_difference(indexList, diffList.begin());
        const auto sum{muc::ranges::reduce(indexList, 0ll)};

        if (n != truthN) {
            PrintError("n != truthN");
        }
        if (not std::ranges::all_of(diffList.cbegin() + 1, diffList.cend(), [](auto d) { return d == 1; })) {
            PrintError("not std::ranges::all_of(diffList.cbegin() + 1, diffList.cend(), [](auto d) { return d == 1; })");
        }
        if (sum != n * (n - 1) / 2) {
            PrintError("sum != n * (n - 1) / 2");
        }
    } else {
        worldComm.gather<int>(0, localIndexList.size());
        worldComm.gatherv(0, localIndexList.data(), mplr::contiguous_layout<int>{localIndexList.size()});
    }
}

auto main(int argc, char* argv[]) -> int {
    Mustard::Env::MPIEnv env{argc, argv, {}};

    Executor<int> executor;

    const auto n{gsl::narrow<int>(std::stoul(argv[1]))};
    const auto worldComm{mplr::comm_world()};

    std::vector<int> localIndexList;
    executor.PrintProgress(false);
    executor(n, [&](auto i) { localIndexList.emplace_back(i); });
    executor.PrintExecutionSummary();
    CheckIndexList(n, localIndexList);
    MasterPrintLn("");

    std::this_thread::sleep_for(1s);

    const auto bigN{std::min<long long>(1000000ll * n, std::numeric_limits<int>::max() / 2)};
    localIndexList.clear();
    executor.PrintProgress(true);
    executor(bigN, [&](auto i) { localIndexList.emplace_back(i); });
    executor.PrintExecutionSummary();
    CheckIndexList(bigN, localIndexList);
    MasterPrintLn("");

    std::this_thread::sleep_for(1s);

    localIndexList.clear();
    executor(n, [&](auto i) {
        localIndexList.emplace_back(i);
        std::this_thread::sleep_for(500ms);
    });
    executor.PrintExecutionSummary();
    CheckIndexList(n, localIndexList);
    MasterPrintLn("");

    std::this_thread::sleep_for(1s);

    executor.PrintProgressInterval(100ms);
    localIndexList.clear();
    executor(n, [&](auto i) {
        localIndexList.emplace_back(i);
        std::this_thread::sleep_for(500ms);
    });
    executor.PrintExecutionSummary();
    CheckIndexList(n, localIndexList);

    return EXIT_SUCCESS;
}
