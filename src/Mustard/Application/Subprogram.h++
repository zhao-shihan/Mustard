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

#include <string>

namespace Mustard::Application {

/// @brief Base class for a subprogram executed by SubprogramLauncher.
///
/// Subprogram represents a named, self-contained application that can be
/// registered with and launched by SubprogramLauncher. Each concrete
/// subprogram must implement Main() to define its own command-line behavior.
///
/// Typical usage: derive from Subprogram, pass name and description to the
/// base constructor, implement Main(), then register via
/// SubprogramLauncher::AddSubprogram().
class Subprogram {
public:
    /// @brief Construct a subprogram with a name and description.
    /// @param name        Short name used as the command-line subcommand.
    /// @param description Description shown in help text.
    Subprogram(std::string name, std::string description);
    /// @brief Virtual destructor.
    virtual ~Subprogram() = default;

    /// @brief Return the subprogram's name.
    auto Name() const -> const auto& { return fName; }
    /// @brief Return the subprogram's description.
    auto Description() const -> const auto& { return fDescription; }

    /// @brief Entry point of the subprogram.
    /// @param argc Number of command-line arguments.
    /// @param argv Command-line argument array.
    /// @return Exit code.
    ///
    /// Derived classes must override this to implement subprogram logic.
    virtual auto Main(int argc, char* argv[]) const -> int = 0;

private:
    std::string fName;        ///< Subcommand name.
    std::string fDescription; ///< Subprogram description text.
};

} // namespace Mustard::Application
