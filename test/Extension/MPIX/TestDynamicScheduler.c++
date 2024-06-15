#include "Mustard/Env/MPIEnv.h++"
#include "Mustard/Env/Print.h++"
#include "Mustard/Extension/MPIX/Execution/DynamicScheduler.h++"
#include "Mustard/Extension/MPIX/Execution/Executor.h++"

#include <string>
#include <thread>

using namespace Mustard;
using namespace std::chrono_literals;

auto main(int argc, char* argv[]) -> int {
    Mustard::Env::MPIEnv env{argc, argv, {}};

    MPIX::Executor<unsigned short> executor{MPIX::ScheduleBy<MPIX::DynamicScheduler>{}};

    const auto n{std::stoll(argv[1])};

    executor.PrintProgress(false);
    executor.Execute(n,
                     [&](auto i) {
                         Env::PrintLn("{},{}", i, env.CommWorldRank());
                     });

    std::this_thread::sleep_for(3s);

    executor.PrintProgress(true);
    executor.PrintProgressModulo(-1);
    executor.Execute(n,
                     [&](auto i) {
                         std::this_thread::sleep_for(500ms);
                         Env::PrintLn("{},{}", i, env.CommWorldRank());
                     });

    executor.Execute(n,
                     [&](auto i) {
                         std::this_thread::sleep_for(500ms);
                         Env::PrintLn("{},{}", i, env.CommWorldRank());
                     });

    executor.PrintProgressModulo(1);
    executor.Execute(n,
                     [&](auto) {
                         std::this_thread::sleep_for(500ms);
                     });

    return EXIT_SUCCESS;
}
