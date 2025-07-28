#include "Mustard/Env/MPIEnv.h++"
#include "Mustard/Extension/MPIX/Execution/ClusterAwareMasterWorkerScheduler.h++"
#include "Mustard/Extension/MPIX/Execution/Executor.h++"
#include "Mustard/Utility/Print.h++"

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

    MPIX::Executor<int> executor;

    const auto n{gsl::narrow<int>(std::stoul(argv[1]))};
    const auto worldComm{mplr::comm_world()};

    std::vector<int> localIndexList;
    executor.PrintProgress(false);
    executor.Execute(n, [&](auto i) { localIndexList.emplace_back(i); });
    executor.PrintExecutionSummary();
    CheckIndexList(n, localIndexList);
    MasterPrintLn("");

    std::this_thread::sleep_for(1s);

    const auto bigN{std::min<long long>(1000000ll * n, std::numeric_limits<int>::max() / 2)};
    localIndexList.clear();
    executor.PrintProgress(true);
    executor.Execute(bigN, [&](auto i) { localIndexList.emplace_back(i); });
    executor.PrintExecutionSummary();
    CheckIndexList(bigN, localIndexList);
    MasterPrintLn("");

    std::this_thread::sleep_for(1s);

    executor.PrintProgress(true);
    executor.PrintProgressModulo(-1);
    localIndexList.clear();
    executor.Execute(n,
                     [&](auto i) {
                         localIndexList.emplace_back(i);
                         std::this_thread::sleep_for(500ms);
                     });
    executor.PrintExecutionSummary();
    CheckIndexList(n, localIndexList);
    MasterPrintLn("");

    std::this_thread::sleep_for(1s);

    localIndexList.clear();
    executor.Execute(n,
                     [&](auto i) {
                         localIndexList.emplace_back(i);
                         std::this_thread::sleep_for(500ms);
                     });
    executor.PrintExecutionSummary();
    CheckIndexList(n, localIndexList);
    MasterPrintLn("");

    std::this_thread::sleep_for(1s);

    executor.PrintProgressModulo(1);
    localIndexList.clear();
    executor.Execute(n,
                     [&](auto i) {
                         localIndexList.emplace_back(i);
                         std::this_thread::sleep_for(500ms);
                     });
    executor.PrintExecutionSummary();
    CheckIndexList(n, localIndexList);

    return EXIT_SUCCESS;
}
