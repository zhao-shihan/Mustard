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

namespace Mustard::Env::CLI::inline Module {

template<muc::ceta_string AName, muc::ceta_string AShortName>
InputFileModule<AName, AShortName>::InputFileModule(argparse::ArgumentParser& argParser) :
    ModuleBase{argParser} {
    ([this]() -> auto& {
        if constexpr (AShortName) {
            return ArgParser().add_argument(AShortName.sv(), AName.sv());
        } else {
            return ArgParser().add_argument(AName.sv());
        }
    }())
        .required()
        .nargs(argparse::nargs_pattern::at_least_one)
        .help("Input file path(s).");
}

} // namespace Mustard::Env::CLI::inline Module
