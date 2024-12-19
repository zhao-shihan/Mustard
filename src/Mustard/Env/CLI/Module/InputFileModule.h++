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

#include "muc/ceta_string"

#include <string>
#include <vector>

namespace Mustard::Env::CLI::inline Module {

template<muc::ceta_string AName, muc::ceta_string AShortName = nullptr>
class InputFileModule : public ModuleBase {
public:
    InputFileModule(argparse::ArgumentParser& argParser);

    auto InputFilePath() const -> auto { return ArgParser().get<std::vector<std::string>>(AName.sv()); }
};

} // namespace Mustard::Env::CLI::inline Module

#include "Mustard/Env/CLI/Module/InputFileModule.inl"
