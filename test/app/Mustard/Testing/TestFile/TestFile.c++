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

#include "mplr/mplr.hpp"

#include <memory>
#include <vector>

namespace Mustard::Testing {

TestFile::TestFile() :
    Subprogram{"TestFile", "Test Mustard::File."} {}

auto TestFile::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    cli->add_argument("path").help("File path.").default_value("test_file").required().nargs(1);
    cli->add_argument("extension").help("File extension.").default_value("root").required().nargs(1);
    Mustard::Env::MPIEnv env{argc, argv, cli};

    const auto path{cli->get("path")};
    const auto extension{cli->get("extension")};

    std::vector<std::unique_ptr<Mustard::File<>>> file;
    if (mplr::comm_world().size() == 1) {
        int i{};
        file.emplace_back(std::make_unique<Mustard::File<TFile>>(fmt::format("{}_{}.{}", path, i++, extension), "RECREATE"));
        file.emplace_back(std::make_unique<Mustard::File<std::FILE>>(fmt::format("{}_{}.{}", path, i++, extension), "w"));
        file.emplace_back(std::make_unique<Mustard::File<std::ifstream>>(fmt::format("{}_{}.{}", path, i++, extension)));
        file.emplace_back(std::make_unique<Mustard::File<std::ofstream>>(fmt::format("{}_{}.{}", path, i++, extension)));
        file.emplace_back(std::make_unique<Mustard::File<std::fstream>>(fmt::format("{}_{}.{}", path, i++, extension)));
        file.emplace_back(std::make_unique<Mustard::File<std::wifstream>>(fmt::format("{}_{}.{}", path, i++, extension)));
        file.emplace_back(std::make_unique<Mustard::File<std::wofstream>>(fmt::format("{}_{}.{}", path, i++, extension)));
        file.emplace_back(std::make_unique<Mustard::File<std::wfstream>>(fmt::format("{}_{}.{}", path, i++, extension)));
    }
    int i{};
    file.emplace_back(std::make_unique<Mustard::ProcessSpecificFile<TFile>>(fmt::format("{}_{}.{}", path, i++, extension), "RECREATE"));
    file.emplace_back(std::make_unique<Mustard::ProcessSpecificFile<std::FILE>>(fmt::format("{}_{}.{}", path, i++, extension), "w"));
    file.emplace_back(std::make_unique<Mustard::ProcessSpecificFile<std::ifstream>>(fmt::format("{}_{}.{}", path, i++, extension)));
    file.emplace_back(std::make_unique<Mustard::ProcessSpecificFile<std::ofstream>>(fmt::format("{}_{}.{}", path, i++, extension)));
    file.emplace_back(std::make_unique<Mustard::ProcessSpecificFile<std::fstream>>(fmt::format("{}_{}.{}", path, i++, extension)));
    file.emplace_back(std::make_unique<Mustard::ProcessSpecificFile<std::wifstream>>(fmt::format("{}_{}.{}", path, i++, extension)));
    file.emplace_back(std::make_unique<Mustard::ProcessSpecificFile<std::wofstream>>(fmt::format("{}_{}.{}", path, i++, extension)));
    file.emplace_back(std::make_unique<Mustard::ProcessSpecificFile<std::wfstream>>(fmt::format("{}_{}.{}", path, i++, extension)));

    file.clear(); // Close files

    if (mplr::comm_world().size() == 1) {
        file.emplace_back(std::make_unique<Mustard::File<std::FILE>>(fmt::format("{}_0.{}", path, extension), "a"));
        file.emplace_back(std::make_unique<Mustard::File<TFile>>(fmt::format("{}_0.{}", path, extension), "UPDATE"));
    }
    file.emplace_back(std::make_unique<Mustard::ProcessSpecificFile<std::FILE>>(fmt::format("{}_0.{}", path, extension), "a"));
    file.emplace_back(std::make_unique<Mustard::ProcessSpecificFile<TFile>>(fmt::format("{}_0.{}", path, extension), "UPDATE"));

    file.clear(); // Close files

    if (mplr::comm_world().size() == 1) {
        file.emplace_back(std::make_unique<Mustard::File<std::FILE>>(fmt::format("{}_0.{}", path, extension), "r"));
        file.emplace_back(std::make_unique<Mustard::File<TFile>>(fmt::format("{}_0.{}", path, extension), "READ"));
    }
    file.emplace_back(std::make_unique<Mustard::ProcessSpecificFile<std::FILE>>(fmt::format("{}_0.{}", path, extension), "r"));
    file.emplace_back(std::make_unique<Mustard::ProcessSpecificFile<TFile>>(fmt::format("{}_0.{}", path, extension), "READ"));

    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
