// -*- C++ -*-
//
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

#pragma once

#include "argparse/argparse.hpp"

#include <memory>

namespace Mustard::CLI::impl {

/// @brief Create the default argument parser with standard flags.
///
/// Returns a unique_ptr to an argparse::ArgumentParser pre-configured
/// with the standard Mustard command-line arguments (e.g. --help).
/// Used by CLI<> and SubprogramLauncher during construction.
///
/// @return A unique_ptr to the configured ArgumentParser.
auto MakeDefaultArgParser() -> std::unique_ptr<argparse::ArgumentParser>;

} // namespace Mustard::CLI::impl
