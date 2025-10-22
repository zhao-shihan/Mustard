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

#include "Mustard/IO/PrettyLog.h++"
#include "Mustard/IO/Print.h++"
#include "Mustard/Utility/FormatToLocalTime.h++"

#include "mplr/mplr.hpp"

#include "fmt/chrono.h"
#include "fmt/color.h"

#include <chrono>
#include <cstdio>
#include <filesystem>

namespace Mustard::inline IO {

namespace internal {
namespace {

auto PrettyLogHead(std::string_view prefix, const std::source_location& location) -> std::string {
    return fmt::format("[{}] {}{}:{}:{} `{}`{}",
                       FormatToLocalTime(std::chrono::system_clock::now()),
                       prefix,
                       std::filesystem::path{location.file_name()}.filename().generic_string(),
                       location.line(),
                       location.column(),
                       location.function_name(),
                       mplr::available() ?
                           fmt::format(" in MPI process {}", mplr::comm_world().rank()) :
                           "");
}

} // namespace

auto PrettyException(std::string_view message, const std::source_location& location) -> std::string {
    return fmt::format("{}: {}", PrettyLogHead("", location), message);
}

} // namespace internal

auto PrintInfo(std::string_view message, const std::source_location& location) -> void {
    const auto ts{fg(fmt::color::deep_sky_blue)};
    Print<'I'>(stderr, ts, "{}: ", internal::PrettyLogHead("Information from ", location));
    Print<'I'>(stderr, ts | fmt::emphasis::bold, "{}", message);
    Print<'I'>(stderr, "\n");
}

auto PrintWarning(std::string_view message, const std::source_location& location) -> void {
    const auto ts{fg(fmt::color::white) | bg(fmt::color::dark_orange)};
    Print<'W'>(stderr, ts | fmt::emphasis::bold | fmt::emphasis::blink, "***");
    Print<'W'>(stderr, ts, " {}: ", internal::PrettyLogHead("Warning from ", location));
    Print<'W'>(stderr, ts | fmt::emphasis::bold, "{}", message);
    Print<'W'>(stderr, "\n");
}

auto PrintError(std::string_view message, const std::source_location& location) -> void {
    const auto ts{fg(fmt::color::white) | bg(fmt::color::tomato)};
    Print<'E'>(stderr, ts | fmt::emphasis::bold | fmt::emphasis::blink, "***");
    Print<'E'>(stderr, ts, " {}: ", internal::PrettyLogHead("Error from ", location));
    Print<'E'>(stderr, ts | fmt::emphasis::bold, "{}", message);
    Print<'E'>(stderr, "\n");
}

auto MasterPrintInfo(std::string_view message, const std::source_location& location) -> void {
    if (not mplr::available() or
        mplr::comm_world().rank() == 0) {
        PrintInfo(message, location);
    }
}

auto MasterPrintWarning(std::string_view message, const std::source_location& location) -> void {
    if (not mplr::available() or
        mplr::comm_world().rank() == 0) {
        PrintWarning(message, location);
    }
}

auto MasterPrintError(std::string_view message, const std::source_location& location) -> void {
    if (not mplr::available() or
        mplr::comm_world().rank() == 0) {
        PrintError(message, location);
    }
}

} // namespace Mustard::inline IO
