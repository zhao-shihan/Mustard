// Copyright (C) 2020-2025  Mustard developers
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
#include "Mustard/IO/CreateTemporaryFile.h++"

#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>

using namespace Mustard;

auto main(int argc, char* argv[]) -> int {
    Mustard::Env::MPIEnv env{argc, argv, {}};

    const auto path{CreateTemporaryFile(argv[1], argv[2])};
    std::cout << path << std::endl;
    if (not std::filesystem::exists(path)) {
        throw std::runtime_error{"bug"};
    }

    std::getchar();

    return EXIT_SUCCESS;
}
