// -*- C++ -*-
//
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

#pragma once

#include "Mustard/Application/Subprogram.h++"

#include <concepts>
#include <memory>

namespace argparse {
class ArgumentParser;
} // namespace argparse

namespace Mustard::Application {

class SubprogramLauncher {
public:
    SubprogramLauncher();
    ~SubprogramLauncher();

    template<std::derived_from<Subprogram> ASubprogram>
    auto AddSubprogram() -> void { AddSubprogram(std::make_unique<ASubprogram>()); }
    auto AddSubprogram(std::unique_ptr<Subprogram> subprogram) -> void;
    auto LaunchMain(int argc, char* argv[]) -> int;

private:
    struct State;
    std::unique_ptr<State> fState;
};

} // namespace Mustard::Application
