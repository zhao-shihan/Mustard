// -*- C++ -*-
//
// Copyright (C) 2020-2025  The Mustard development team
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

#include "Mustard/CLI/Module/BasicModule.h++"
#include "Mustard/Env/BasicEnv.h++"
#include "Mustard/IO/Print.h++"
#include "Mustard/Utility/FormatToLocalTime.h++"
#include "Mustard/Version.h++"

#include "TThread.h"

#include "fmt/chrono.h"
#include "fmt/color.h"

#include <chrono>
#include <filesystem>
#include <system_error>
#include <typeinfo>

namespace Mustard::Env {

BasicEnv::BasicEnv(NoBanner, int argc, char* argv[],
                   muc::optional_ref<CLI::CLI<>> cli,
                   enum VerboseLevel verboseLevel,
                   bool showBannerHint) :
    EnvBase{},
    PassiveSingleton{this},
    fShowBanner{showBannerHint},
    fArgc{argc},
    fArgv{argv},
    fVerboseLevel{verboseLevel} {
    // CLI: do parse and get args
    if (cli) {
        const auto pCLI{&cli->get()};
        pCLI->ParseArgs(argc, argv);
        if (const auto basicCLI{dynamic_cast<const CLI::BasicModule*>(pCLI)}) {
            fVerboseLevel = basicCLI->VerboseLevel().value_or(verboseLevel);
            fShowBanner = basicCLI->ShowBanner();
        }
    }

    // We have async ROOT IO, enable ROOT global mutex
    TThread::Initialize();
}

BasicEnv::BasicEnv(int argc, char* argv[],
                   muc::optional_ref<CLI::CLI<>> cli,
                   enum VerboseLevel verboseLevel,
                   bool showBannerHint) :
    BasicEnv{{}, argc, argv, cli, verboseLevel, showBannerHint} {
    if (fShowBanner) {
        PrintStartBannerSplitLine();
        PrintStartBannerBody(argc, argv);
        PrintStartBannerSplitLine();
    }
}

BasicEnv::~BasicEnv() {
    if (fShowBanner) {
        PrintExitBanner();
    }
    fShowBanner = false;
}

auto BasicEnv::PrintExitBanner() const -> void {
    Print(fmt::emphasis::bold,
          "===============================================================================\n"
          " Exit Mustard environment at {}\n"
          "===============================================================================",
          FormatToLocalTime(std::chrono::system_clock::now()));
    Print("\n");
}

auto BasicEnv::PrintStartBannerSplitLine() const -> void {
    Print(fmt::emphasis::bold,
          "\n===============================================================================");
    Print("\n");
}

auto BasicEnv::PrintStartBannerBody(int argc, char* argv[]) const -> void {
    std::error_code cwdError;
    const auto exe{std::filesystem::path(argv[0]).filename().generic_string()};
    auto cwd{std::filesystem::current_path(cwdError).generic_string()};
    if (cwdError) {
        cwd = "<Error getting current working directory>";
    }
    Print(fmt::emphasis::bold,
          " ______  ___             _____              _________\n"
          " ___   |/  /___  __________  /______ _____________  /\n"
          " __  /|_/ /_  / / /_  ___/  __/  __ `/_  ___/  __  / \n"
          " _  /  / / / /_/ /_(__  )/ /_ / /_/ /_  /   / /_/ /  Version\n"
          " /_/  /_/  \\____/ /____/ \\__/ \\____/ /_/    \\____/   " MUSTARD_VERSION_STRING "\n"
          "\n");
    Print(fmt::emphasis::bold | fmt::emphasis::italic,
          " An offline software framework for HEP experiments\n");
    Print(fmt::emphasis::bold,
          " Copyright (C) 2020-2025  The Mustard development team\n"
          "\n"
          " Start at {}\n"
          " Exe: {}",
          FormatToLocalTime(std::chrono::system_clock::now()),
          exe);
    for (auto i{1}; i < argc; ++i) {
        Print(fmt::emphasis::bold, " {}", argv[i]);
    }
    Print(fmt::emphasis::bold,
          "\n"
          " CWD: {}\n",
          cwd);
    Print<'I'>("\n"
               " List of all {} command line arguments:\n",
               argc);
    for (int i{}; i < argc; ++i) {
        PrintLn<'I'>("  argv[{}]: {}", i, argv[i]);
    }
}

} // namespace Mustard::Env
