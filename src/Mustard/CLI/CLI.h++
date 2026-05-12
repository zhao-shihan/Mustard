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

#include "Mustard/CLI/Module/ModuleBase.h++"

#include "argparse/argparse.hpp"

#include "muc/type_traits"

#include <concepts>
#include <memory>
#include <optional>
#include <string_view>
#include <utility>

namespace Mustard::CLI {

/// @brief Variadic CLImate template for composing CLI modules.
///
/// CLI is a variadic class template that combines argument parsing with a
/// set of user-selectable modules. Each module (derived from ModuleBase)
/// registers its own command-line arguments and provides query methods.
///
/// Two specializations:
/// - CLI<> : base class that owns the argument parser and handles parsing.
/// - CLI<AModules...> : inherits from CLI<> and all specified modules.
///
/// @tparam AModules  Pack of ModuleBase-derived types; must be unique.
template<std::derived_from<ModuleBase>... AModules>
    requires muc::is_type_set_v<AModules...>
class CLI;

/// @brief Empty CLI specialization that owns the argument parser.
///
/// Provides the core argument parsing infrastructure: parse command-line
/// arguments, query the parser, and access argument values. Can only be
/// parsed once — repeated calls to ParseArgs() will throw.
///
/// This class uses operator-> and operator[] to expose the underlying
/// argparse::ArgumentParser directly.
template<>
class CLI<> {
public:
    /// @brief Construct with default argument parser.
    CLI();
    /// @brief Virtual destructor.
    virtual ~CLI() = 0;

public:
    /// @brief Parse command-line arguments.
    /// @param argc Argument count.
    /// @param argv Argument vector.
    /// @throws std::logic_error if ParseArgs() has already been called.
    auto ParseArgs(int argc, char* argv[]) -> void;

    /// @brief Check whether arguments have been parsed.
    auto Parsed() const -> bool { return fArgcArgv.has_value(); }

    /// @brief Get the original (argc, argv) pair.
    /// @return A pair of argument count and argument vector.
    auto ArgcArgv() const -> std::pair<int, char**>;

    /// @brief Pointer access to the underlying argument parser.
    auto operator->() const -> const auto* { return fArgParser.get(); }
    /// @brief Mutable pointer access to the underlying argument parser.
    auto operator->() -> auto* { return fArgParser.get(); }

    /// @brief Access a parsed argument by name.
    /// @param arg  Argument name (e.g. "--verbose").
    /// @return Reference to the argument value.
    auto operator[](std::string_view arg) -> auto& { return (*fArgParser)[arg]; }

protected:
    /// @brief Const access to the argument parser.
    auto ArgParser() const -> const auto& { return *fArgParser; }
    /// @brief Mutable access to the argument parser.
    auto ArgParser() -> auto& { return *fArgParser; }

protected:
    /// @brief Throw if ParseArgs() has already been called.
    [[noreturn]] static auto ThrowParsed() -> void;
    /// @brief Throw if ParseArgs() has not yet been called.
    [[noreturn]] static auto ThrowNotParsed() -> void;

private:
    std::optional<std::pair<int, char**>> fArgcArgv;      ///< Original (argc, argv) stored after parsing.
    std::unique_ptr<argparse::ArgumentParser> fArgParser; ///< The underlying argument parser instance.
};

/// @brief Variadic CLI specialization that inherits from all modules.
///
/// Inherits from CLI<> for parsing and from each module in AModules...
/// for domain-specific CLI functionality. The constructor passes a pointer
/// to the CLI instance to each module's constructor.
///
/// @tparam AModules  Pack of unique ModuleBase-derived types.
template<std::derived_from<ModuleBase>... AModules>
    requires muc::is_type_set_v<AModules...>
class CLI : public CLI<>,
            public AModules... {
public:
    /// @brief Construct the CLI and all its modules.
    CLI();
};

} // namespace Mustard::CLI

#include "Mustard/CLI/CLI.inl"
