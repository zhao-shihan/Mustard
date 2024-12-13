#include "Mustard/Env/MPIEnv.h++"
#include "Mustard/Extension/MPIX/Execution/DynamicScheduler.h++"
#include "Mustard/Extension/MPIX/Execution/Executor.h++"
#include "Mustard/Utility/Print.h++"

#include <string>
#include <thread>

using namespace Mustard;
using namespace std::chrono_literals;

auto main(int argc, char* argv[]) -> int {
    Mustard::Env::MPIEnv env{argc, argv, {}};

    MPIX::Executor<unsigned long long> executor{MPIX::ScheduleBy<MPIX::DynamicScheduler>{}};

    const auto n{std::stoull(argv[1])};

    executor.PrintProgress(false);
    executor.Execute(n,
                     [&](auto i) {
                         PrintLn("{},{}", i, env.CommWorldRank());
                     });

    executor.PrintProgress(true);
    executor.Execute(1000000000ull * n, [&](auto) {});

    std::this_thread::sleep_for(3s);

    executor.PrintProgress(true);
    executor.PrintProgressModulo(-1);
    executor.Execute(n,
                     [&](auto i) {
                         std::this_thread::sleep_for(500ms);
                         PrintLn("{},{}", i, env.CommWorldRank());
                     });

    executor.Execute(n,
                     [&](auto i) {
                         std::this_thread::sleep_for(500ms);
                         PrintLn("{},{}", i, env.CommWorldRank());
                     });

    executor.PrintProgressModulo(1);
    executor.Execute(n,
                     [&](auto) {
                         std::this_thread::sleep_for(500ms);
                     });

    return EXIT_SUCCESS;
}
