#include "Mustard/Env/BasicEnv.h++"
#include "Mustard/Execution/Executor.h++"

#include "gsl/gsl"

#include <chrono>
#include <string>
#include <thread>

using namespace Mustard;

auto main(int argc, char* argv[]) -> int {
    Mustard::Env::BasicEnv env{argc, argv, {}};

    Executor<unsigned long long> executor;
    executor.Execute(std::stoull(argv[1]),
                     [](auto) {
                        //  using std::chrono_literals::operator""ms;
                        //  std::this_thread::sleep_for(1ms);
                     });
    executor.PrintExecutionSummary();

    return EXIT_SUCCESS;
}
