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
#include "Mustard/IO/CreateTemporaryFile.h++"
#include "Mustard/IO/PrettyLog.h++"
#include "Mustard/IO/Print.h++"
#include "Mustard/Testing/TestCreateTemporaryFile/TestCreateTemporaryFile.h++"

#include "fmt/std.h"

#include <filesystem>
#include <stdexcept>
#include <string>

namespace Mustard::Testing {

TestCreateTemporaryFile::TestCreateTemporaryFile() :
    Subprogram{"TestCreateTemporaryFile", "Test Mustard::CreateTemporaryFile."} {}

auto TestCreateTemporaryFile::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    cli->add_argument("pattern").help("Temporary file pattern.").nargs(1);
    cli->add_argument("prefix").help("Temporary file prefix.").nargs(1);
    Mustard::Env::MPIEnv env{argc, argv, cli};

    const auto path{CreateTemporaryFile(cli->get("pattern"), cli->get("prefix"))};
    PrintLn("{}", path);
    if (not std::filesystem::exists(path)) {
        Throw<std::runtime_error>("bug");
    }

    std::getchar();

    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
