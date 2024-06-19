// -*- C++ -*-
//
// Copyright 2020-2024  The Mustard development team
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

#pragma once

#include "Mustard/Env/CLI/CLI.h++"
#include "Mustard/Env/Memory/PassiveSingleton.h++"
#include "Mustard/Env/VerboseLevel.h++"
#include "Mustard/Env/internal/EnvBase.h++"
#include "Mustard/Utility/InlineMacro.h++"

#include "fmt/format.h"

#include <functional>
#include <optional>

namespace Mustard::Env {

class BasicEnv : public internal::EnvBase,
                 public Memory::PassiveSingleton<BasicEnv> {
public:
    BasicEnv(int argc, char* argv[],
             std::optional<std::reference_wrapper<CLI::CLI<>>> cli = {},
             enum VerboseLevel verboseLevel = {},
             bool printWelcomeMessage = true);
    virtual ~BasicEnv();

    auto Argc() const -> auto { return fArgc; }
    auto Argv() const -> auto { return fArgv; }
    auto VerboseLevel() const -> auto { return fVerboseLevel; }

protected:
    auto PrintWelcomeMessageSplitLine() const -> void;
    auto PrintWelcomeMessageBody(int argc, char* argv[]) const -> void;

private:
    int fArgc;
    char** fArgv;
    enum VerboseLevel fVerboseLevel;
};

template<char L>
    requires(L == 'E' or L == 'W' or L == 'I' or L == 'V')
MUSTARD_ALWAYS_INLINE auto VerboseLevelReach() -> bool {
    if (not BasicEnv::Available()) [[unlikely]] { return true; }
    const auto vl{BasicEnv::Instance().VerboseLevel()};
    // if constexpr (L == 'Q') { return vl >= VerboseLevel::Quiet; }
    if constexpr (L == 'E') { return vl >= VerboseLevel::Error; }
    if constexpr (L == 'W') { return vl >= VerboseLevel::Warning; }
    if constexpr (L == 'I') { return vl >= VerboseLevel::Informative; }
    if constexpr (L == 'V') { return vl >= VerboseLevel::Verbose; }
}

} // namespace Mustard::Env
