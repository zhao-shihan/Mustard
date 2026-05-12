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
#include "Mustard/IO/File.h++"
#include "Mustard/Testing/TestFile/TestFile.h++"

#include <memory>
#include <vector>

namespace Mustard::Testing {

TestFile::TestFile() :
    Subprogram{"TestFile", "Test Mustard::File."} {}

auto TestFile::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    cli->add_argument("path").help("File path.").nargs(1);
    Mustard::Env::MPIEnv env{argc, argv, cli};

    const auto path{cli->get("path")};

    std::vector<std::unique_ptr<Mustard::File<>>> file;
    file.emplace_back(std::make_unique<Mustard::File<std::FILE>>(path, "w"));
    file.emplace_back(std::make_unique<Mustard::File<std::FILE>>(path, "r"));
    file.emplace_back(std::make_unique<Mustard::File<std::FILE>>(path, "a"));
    file.emplace_back(std::make_unique<Mustard::File<std::ifstream>>(path));
    file.emplace_back(std::make_unique<Mustard::File<std::ofstream>>(path));
    file.emplace_back(std::make_unique<Mustard::File<std::fstream>>(path));
    file.emplace_back(std::make_unique<Mustard::File<std::wifstream>>(path));
    file.emplace_back(std::make_unique<Mustard::File<std::wofstream>>(path));
    file.emplace_back(std::make_unique<Mustard::File<std::wfstream>>(path));
    file.emplace_back(std::make_unique<Mustard::File<TFile>>(path, "READ"));
    file.emplace_back(std::make_unique<Mustard::File<TFile>>(path, "UPDATE"));
    file.emplace_back(std::make_unique<Mustard::File<TFile>>(path, "RECREATE"));

    file.emplace_back(std::make_unique<Mustard::ProcessSpecificFile<std::FILE>>(path, "w"));
    file.emplace_back(std::make_unique<Mustard::ProcessSpecificFile<std::FILE>>(path, "r"));
    file.emplace_back(std::make_unique<Mustard::ProcessSpecificFile<std::FILE>>(path, "a"));
    file.emplace_back(std::make_unique<Mustard::ProcessSpecificFile<std::ifstream>>(path));
    file.emplace_back(std::make_unique<Mustard::ProcessSpecificFile<std::ofstream>>(path));
    file.emplace_back(std::make_unique<Mustard::ProcessSpecificFile<std::fstream>>(path));
    file.emplace_back(std::make_unique<Mustard::ProcessSpecificFile<std::wifstream>>(path));
    file.emplace_back(std::make_unique<Mustard::ProcessSpecificFile<std::wofstream>>(path));
    file.emplace_back(std::make_unique<Mustard::ProcessSpecificFile<std::wfstream>>(path));
    file.emplace_back(std::make_unique<Mustard::ProcessSpecificFile<TFile>>(path, "READ"));
    file.emplace_back(std::make_unique<Mustard::ProcessSpecificFile<TFile>>(path, "UPDATE"));
    file.emplace_back(std::make_unique<Mustard::ProcessSpecificFile<TFile>>(path, "RECREATE"));

    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
