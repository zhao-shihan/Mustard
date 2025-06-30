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

#include "Mustard/Utility/PrettyLog.h++"
#include "Mustard/Utility/Print.h++"

#include "mpl/mpl.hpp"

#include "muc/time"

#include "fmt/chrono.h"
#include "fmt/color.h"

#include <chrono>
#include <filesystem>

namespace Mustard::inline Utility {

namespace internal {
namespace {

auto PrettyLogHead(std::string_view prefix, const std::source_location& location) -> std::string {
    using scsc = std::chrono::system_clock;
    return fmt::format("[{:%FT%T%z}] {}{}:{}:{} `{}`{}",
                       muc::localtime(scsc::to_time_t(scsc::now())),
                       prefix,
                       std::filesystem::path{location.file_name()}.filename().generic_string(),
                       location.line(),
                       location.column(),
                       location.function_name(),
                       mpl::environment::available() ?
                           fmt::format(" in MPI process {}", mpl::environment::comm_world().rank()) :
                           "");
}

} // namespace
} // namespace internal

auto PrintInfo(std::string_view message, const std::source_location& location) -> void {
    const auto ts{fg(fmt::color::deep_sky_blue)};
    Print<'I'>(ts, "{}: ", internal::PrettyLogHead("Information from ", location));
    Print<'I'>(ts | fmt::emphasis::bold, "{}", message);
    Print<'I'>("\n");
}

auto PrintWarning(std::string_view message, const std::source_location& location) -> void {
    const auto ts{fg(fmt::color::white) | bg(fmt::color::dark_orange)};
    Print<'W'>(ts | fmt::emphasis::bold | fmt::emphasis::blink, "***");
    Print<'W'>(ts, " {}: ", internal::PrettyLogHead("Warning from ", location));
    Print<'W'>(ts | fmt::emphasis::bold, "{}", message);
    Print<'W'>("\n");
}

auto PrintError(std::string_view message, const std::source_location& location) -> void {
    const auto ts{fg(fmt::color::white) | bg(fmt::color::red)};
    Print<'E'>(ts | fmt::emphasis::bold | fmt::emphasis::blink, "***");
    Print<'E'>(ts, " {}: ", internal::PrettyLogHead("Error from ", location));
    Print<'E'>(ts | fmt::emphasis::bold, "{}", message);
    Print<'E'>("\n");
}

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
