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
#include "Mustard/IO/File.h++"

#include <memory>
#include <vector>

auto main(int argc, char* argv[]) -> int {
    Mustard::Env::MPIEnv env{argc, argv, {}};

    std::vector<std::unique_ptr<Mustard::File<>>> file;
    file.emplace_back(std::make_unique<Mustard::File<std::FILE>>(argv[1], "w"));
    file.emplace_back(std::make_unique<Mustard::File<std::FILE>>(argv[1], "r"));
    file.emplace_back(std::make_unique<Mustard::File<std::FILE>>(argv[1], "a"));
    file.emplace_back(std::make_unique<Mustard::File<std::ifstream>>(argv[1]));
    file.emplace_back(std::make_unique<Mustard::File<std::ofstream>>(argv[1]));
    file.emplace_back(std::make_unique<Mustard::File<std::fstream>>(argv[1]));
    file.emplace_back(std::make_unique<Mustard::File<std::wifstream>>(argv[1]));
    file.emplace_back(std::make_unique<Mustard::File<std::wofstream>>(argv[1]));
    file.emplace_back(std::make_unique<Mustard::File<std::wfstream>>(argv[1]));
    file.emplace_back(std::make_unique<Mustard::File<TFile>>(argv[1], "READ"));
    file.emplace_back(std::make_unique<Mustard::File<TFile>>(argv[1], "UPDATE"));
    file.emplace_back(std::make_unique<Mustard::File<TFile>>(argv[1], "RECREATE"));

    file.emplace_back(std::make_unique<Mustard::ProcessSpecificFile<std::FILE>>(argv[1], "w"));
    file.emplace_back(std::make_unique<Mustard::ProcessSpecificFile<std::FILE>>(argv[1], "r"));
    file.emplace_back(std::make_unique<Mustard::ProcessSpecificFile<std::FILE>>(argv[1], "a"));
    file.emplace_back(std::make_unique<Mustard::ProcessSpecificFile<std::ifstream>>(argv[1]));
    file.emplace_back(std::make_unique<Mustard::ProcessSpecificFile<std::ofstream>>(argv[1]));
    file.emplace_back(std::make_unique<Mustard::ProcessSpecificFile<std::fstream>>(argv[1]));
    file.emplace_back(std::make_unique<Mustard::ProcessSpecificFile<std::wifstream>>(argv[1]));
    file.emplace_back(std::make_unique<Mustard::ProcessSpecificFile<std::wofstream>>(argv[1]));
    file.emplace_back(std::make_unique<Mustard::ProcessSpecificFile<std::wfstream>>(argv[1]));
    file.emplace_back(std::make_unique<Mustard::ProcessSpecificFile<TFile>>(argv[1], "READ"));
    file.emplace_back(std::make_unique<Mustard::ProcessSpecificFile<TFile>>(argv[1], "UPDATE"));
    file.emplace_back(std::make_unique<Mustard::ProcessSpecificFile<TFile>>(argv[1], "RECREATE"));

    return EXIT_SUCCESS;
}
