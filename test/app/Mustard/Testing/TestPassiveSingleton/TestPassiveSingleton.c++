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
#include "Mustard/Env/ObjectRegistry/PassiveSingleton.h++"
#include "Mustard/Testing/TestPassiveSingleton/TestPassiveSingleton.h++"

namespace Mustard::Testing {

namespace {

class NullPassiveSingleton : public Mustard::Env::PassiveSingleton<NullPassiveSingleton> {};

} // namespace

TestPassiveSingleton::TestPassiveSingleton() :
    Subprogram{"TestPassiveSingleton", "Test Mustard::Env::PassiveSingleton."} {}

auto TestPassiveSingleton::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    Mustard::Env::MPIEnv env{argc, argv, cli};

    NullPassiveSingleton::Instance();
    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
