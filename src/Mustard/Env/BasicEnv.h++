// -*- C++ -*-
//
// Copyright 2020-2025  The Mustard development team
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

#include "Mustard/CLI/CLI.h++"
#include "Mustard/Env/Memory/PassiveSingleton.h++"
#include "Mustard/Env/VerboseLevel.h++"
#include "Mustard/Env/internal/EnvBase.h++"
#include "Mustard/Utility/InlineMacro.h++"

#include "muc/optional"

#include "fmt/format.h"

#include <functional>
#include <optional>

namespace Mustard::Env {

class BasicEnv : public virtual internal::EnvBase,
                 public Memory::PassiveSingleton<BasicEnv> {
protected:
    struct NoBanner {};

protected:
    BasicEnv(NoBanner, int& argc, char**& argv,
             muc::optional_ref<CLI::CLI<>> cli,
             enum VerboseLevel verboseLevel,
             bool showBannerHint);

public:
    BasicEnv(int argc, char* argv[],
             muc::optional_ref<CLI::CLI<>> cli = {},
             enum VerboseLevel verboseLevel = {},
             bool showBannerHint = true);

    virtual ~BasicEnv();

    auto Argc() const -> auto { return fArgc; }
    auto Argv() const -> auto { return fArgv; }

    template<char L>
        requires(L == 'E' or L == 'W' or L == 'I' or L == 'V')
    MUSTARD_ALWAYS_INLINE auto VerboseLevelReach() const -> bool;
    auto VerboseLevel() const -> auto { return fVerboseLevel; }

protected:
    auto PrintStartBannerSplitLine() const -> void;
    auto PrintStartBannerBody(int argc, char* argv[]) const -> void;
    auto PrintExitBanner() const -> void;

protected:
    bool fShowBanner;

private:
    int fArgc;
    char** fArgv;
    enum VerboseLevel fVerboseLevel;
};

template<char L>
    requires(L == 'E' or L == 'W' or L == 'I' or L == 'V')
MUSTARD_ALWAYS_INLINE auto VerboseLevelReach() -> bool;

} // namespace Mustard::Env

#include "Mustard/Env/BasicEnv.inl"
