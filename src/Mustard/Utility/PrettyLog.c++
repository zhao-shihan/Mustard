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

#include "Mustard/Env/MPIEnv.h++"
#include "Mustard/Utility/PrettyLog.h++"

#include "fmt/chrono.h"

#include <chrono>

namespace Mustard::inline Utility {

namespace internal {
namespace {

auto PrettyLogHead(std::string_view prefix, const std::source_location& location) -> std::string {
    using scsc = std::chrono::system_clock;
    return fmt::format("[{:%FT%T%z}] {}{}({}:{}) `{}`{}",
                       fmt::localtime(scsc::to_time_t(scsc::now())),
                       prefix,
                       location.file_name(),
                       location.line(),
                       location.column(),
                       location.function_name(),
                       Env::MPIEnv::Available() ?
                           fmt::format(" in MPI process {}", Env::MPIEnv::Instance().CommWorldRank()) :
                           "");
}

} // namespace
} // namespace internal

auto PrettyInfo(std::string_view message, const std::source_location& location) -> std::string {
    return fmt::format("{}: {}", internal::PrettyLogHead("Information from ", location), message);
}

auto PrettyWarning(std::string_view message, const std::source_location& location) -> std::string {
    return fmt::format("{}: {}", internal::PrettyLogHead("Warning from ", location), message);
}

auto PrettyError(std::string_view message, const std::source_location& location) -> std::string {
    return fmt::format("{}: {}", internal::PrettyLogHead("Error from ", location), message);
}

auto PrettyException(std::string_view message, const std::source_location& location) -> std::string {
    return fmt::format("{}: {}", internal::PrettyLogHead("", location), message);
}

} // namespace Mustard::inline Utility
