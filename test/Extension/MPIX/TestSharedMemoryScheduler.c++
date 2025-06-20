#include "Mustard/Env/MPIEnv.h++"
#include "Mustard/Extension/MPIX/Execution/Executor.h++"
#include "Mustard/Extension/MPIX/Execution/SharedMemoryScheduler.h++"
#include "Mustard/Utility/Print.h++"

#include <string>
#include <thread>

using namespace Mustard;
using namespace std::chrono_literals;

auto main(int argc, char* argv[]) -> int {
    Mustard::Env::MPIEnv env{argc, argv, {}};

    MPIX::Executor<unsigned long long> executor{MPIX::ScheduleBy<MPIX::SharedMemoryScheduler>{}};

    const auto n{std::stoull(argv[1])};
    const auto& worldComm{mpl::environment::comm_world()};

    executor.PrintProgress(false);
    executor.Execute(n,
                     [&](auto i) {
                         PrintLn("{},{}", i, worldComm.rank());
                     });

    executor.PrintProgress(true);
    executor.Execute(100000000ull * n, [](auto) {});
    executor.PrintExecutionSummary();

    std::this_thread::sleep_for(1s);

    executor.PrintProgress(true);
    executor.PrintProgressModulo(-1);
    executor.Execute(n,
                     [&](auto i) {
                         std::this_thread::sleep_for(500ms);
                         PrintLn("{},{}", i, worldComm.rank());
                     });
    executor.PrintExecutionSummary();

    executor.Execute(n,
                     [&](auto i) {
                         std::this_thread::sleep_for(500ms);
                         PrintLn("{},{}", i, worldComm.rank());
                     });
    executor.PrintExecutionSummary();

    executor.PrintProgressModulo(1);
    executor.Execute(n,
                     [&](auto) {
                         std::this_thread::sleep_for(500ms);
                     });
    executor.PrintExecutionSummary();

    return EXIT_SUCCESS;
}
