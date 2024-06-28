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

#include "Mustard/Env/CLI/Module/ModuleBase.h++"
#include "Mustard/Utility/NonMoveableBase.h++"

#include "argparse/argparse.hpp"

#include "muc/type_traits"

#include <concepts>
#include <memory>
#include <optional>
#include <utility>

namespace Mustard::Env::CLI {

template<std::derived_from<ModuleBase>... AModules>
    requires muc::is_type_set_v<AModules...>
class CLI;

template<>
class CLI<> : public NonMoveableBase {
public:
    CLI();
    virtual ~CLI() = 0;

private:
    struct ArgcArgvType {
        int argc;
        char** argv;
    };

public:
    auto ParseArgs(int argc, char* argv[]) -> void;
    auto Parsed() const -> bool { return fArgcArgv.has_value(); }
    auto ArgcArgv() const -> ArgcArgvType;

protected:
    auto ArgParser() const -> const auto& { return *fArgParser; }
    auto ArgParser() -> auto& { return *fArgParser; }

protected:
    [[noreturn]] static auto ThrowParsed() -> void;
    [[noreturn]] static auto ThrowNotParsed() -> void;

private:
    std::optional<ArgcArgvType> fArgcArgv;
    std::unique_ptr<argparse::ArgumentParser> fArgParser;
};

template<std::derived_from<ModuleBase>... AModules>
    requires muc::is_type_set_v<AModules...>
class CLI : public CLI<>,
            public AModules... {
public:
    CLI();
};

} // namespace Mustard::Env::CLI

#include "Mustard/Env/CLI/CLI.inl"
