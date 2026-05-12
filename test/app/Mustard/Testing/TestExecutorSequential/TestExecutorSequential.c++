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
#include "Mustard/Env/BasicEnv.h++"
#include "Mustard/Execution/Executor.h++"
#include "Mustard/Testing/TestExecutorSequential/TestExecutorSequential.h++"

#include "gsl/gsl"

#include <chrono>
#include <string>
#include <thread>

namespace Mustard::Testing {

TestExecutorSequential::TestExecutorSequential() :
    Subprogram{"TestExecutorSequential", "Test Mustard::Executor in sequential run."} {}

auto TestExecutorSequential::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    cli->add_argument("n").help("Number of tasks.").default_value(1'000'000ull).required().nargs(1).scan<'i', unsigned long long>();
    Mustard::Env::BasicEnv env{argc, argv, cli};

    Executor<unsigned long long> executor;
    executor.Run(cli->get<unsigned long long>("n"),
                 [](auto) {
                     // using std::chrono_literals::operator""ms;
                     // std::this_thread::sleep_for(1ms);
                 });
    executor.PrintExecutionSummary();

    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
