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

#include "Mustard/IO/PrettyLog.h++"
#include "Mustard/IO/Print.h++"
#include "Mustard/Utility/FormatToLocalTime.h++"
#include "Mustard/Utility/ProcessTag.h++"

#include "mp1/mp1.hpp"

#include "fmt/chrono.h"
#include "fmt/color.h"

#include <chrono>
#include <cstdio>
#include <filesystem>
#include <string>
#include <string_view>

namespace Mustard::inline IO {

namespace impl {
namespace {

auto PrettyLogHead(std::string_view prefix, const std::source_location& location) -> std::string {
    return fmt::format("[{}] {}{}:{}:{} `{}`",
                       FormatToLocalTime(std::chrono::system_clock::now()), prefix,
                       std::filesystem::path{location.file_name()}.filename().generic_string(),
                       location.line(), location.column(), location.function_name());
}

auto PrettyProcessTag() -> std::string {
    return mp1::available() ? fmt::format("{}> ", ProcessTag()) : std::string{};
}

auto PrettyFlush(std::FILE* f) -> void {
    std::fflush(f);
}

auto PrettyFlush(std::ostream& os) -> void {
    os.flush();
}

auto PrettyWrite(std::FILE* f, std::string_view text) -> void {
    std::fwrite(text.data(), 1, text.size(), f);
}

auto PrettyWrite(std::ostream& os, std::string_view text) -> void {
    os.write(text.data(), static_cast<std::streamsize>(text.size()));
}

auto PrettyAppendMessage(std::string& log, fmt::text_style ts, std::string_view procTag, std::string_view message) -> void {
    const auto messageTs{ts | fmt::emphasis::bold};
    auto rest{message};
    while (true) {
        const auto newline{rest.find('\n')};
        const auto line{rest.substr(0, newline)};
        if (not line.empty()) {
            log += fmt::format(messageTs, "{}", line);
        }
        if (newline == std::string_view::npos) {
            break;
        }
        log += '\n';
        rest.remove_prefix(newline + 1);
        if (not procTag.empty()) {
            log += fmt::format(ts, "{}", procTag);
        }
    }
    log += '\n';
}

template<typename AStream>
auto PrettyPrintInfo(AStream& os, std::string_view message, const std::source_location& location) -> void {
    if (not Env::VerboseLevelReach<'I'>()) {
        return;
    }
    const auto ts{fg(fmt::color::deep_sky_blue)};
    const auto procTag{PrettyProcessTag()};
    std::string log;
    log.reserve(message.size() + 128);
    log += fmt::format(ts, "{}{}: ", procTag, PrettyLogHead("Information from ", location));
    PrettyAppendMessage(log, ts, procTag, message);
    PrettyWrite(os, log);
    PrettyFlush(os);
}

template<typename AStream>
auto PrettyPrintWarning(AStream& os, std::string_view message, const std::source_location& location) -> void {
    if (not Env::VerboseLevelReach<'W'>()) {
        return;
    }
    const auto ts{fg(fmt::color::white) | bg(fmt::color::dark_orange)};
    const auto procTag{PrettyProcessTag()};
    std::string log;
    log.reserve(message.size() + 128);
    if (not procTag.empty()) {
        log += fmt::format(ts, "{}", procTag);
    }
    log += fmt::format(ts | fmt::emphasis::bold | fmt::emphasis::blink, "***");
    log += fmt::format(ts, " {}: ", PrettyLogHead("Warning from ", location));
    PrettyAppendMessage(log, ts, procTag, message);
    PrettyWrite(os, log);
    PrettyFlush(os);
}

template<typename AStream>
auto PrettyPrintError(AStream& os, std::string_view message, const std::source_location& location) -> void {
    if (not Env::VerboseLevelReach<'E'>()) {
        return;
    }
    const auto ts{fg(fmt::color::white) | bg(fmt::color::tomato)};
    const auto procTag{PrettyProcessTag()};
    std::string log;
    log.reserve(message.size() + 128);
    if (not procTag.empty()) {
        log += fmt::format(ts, "{}", procTag);
    }
    log += fmt::format(ts | fmt::emphasis::bold | fmt::emphasis::blink, "***");
    log += fmt::format(ts, " {}: ", PrettyLogHead("Error from ", location));
    PrettyAppendMessage(log, ts, procTag, message);
    PrettyWrite(os, log);
    PrettyFlush(os);
}

} // namespace

auto PrettyException(std::string_view message, const std::source_location& location) -> std::string {
    return fmt::format("{}{}: {}",
                       PrettyLogHead("", location),
                       mp1::available() ? fmt::format(" on {}", ProcessTag()) : "",
                       message);
}

} // namespace impl

auto PrintInfo(std::FILE* f, std::string_view message, const std::source_location& location) -> void {
    impl::PrettyPrintInfo(f, message, location);
}

auto PrintInfo(std::ostream& os, std::string_view message, const std::source_location& location) -> void {
    impl::PrettyPrintInfo(os, message, location);
}

auto PrintInfo(std::string_view message, const std::source_location& location) -> void {
    PrintInfo(stderr, message, location);
}

auto PrintWarning(std::FILE* f, std::string_view message, const std::source_location& location) -> void {
    impl::PrettyPrintWarning(f, message, location);
}

auto PrintWarning(std::ostream& os, std::string_view message, const std::source_location& location) -> void {
    impl::PrettyPrintWarning(os, message, location);
}

auto PrintWarning(std::string_view message, const std::source_location& location) -> void {
    PrintWarning(stderr, message, location);
}

auto PrintError(std::FILE* f, std::string_view message, const std::source_location& location) -> void {
    impl::PrettyPrintError(f, message, location);
}

auto PrintError(std::ostream& os, std::string_view message, const std::source_location& location) -> void {
    impl::PrettyPrintError(os, message, location);
}

auto PrintError(std::string_view message, const std::source_location& location) -> void {
    PrintError(stderr, message, location);
}

auto MasterPrintInfo(std::FILE* f, std::string_view message, const std::source_location& location) -> void {
    if (impl::OnMasterProcess()) {
        PrintInfo(f, message, location);
    }
}

auto MasterPrintInfo(std::ostream& os, std::string_view message, const std::source_location& location) -> void {
    if (impl::OnMasterProcess()) {
        PrintInfo(os, message, location);
    }
}

auto MasterPrintInfo(std::string_view message, const std::source_location& location) -> void {
    if (impl::OnMasterProcess()) {
        PrintInfo(message, location);
    }
}

auto MasterPrintWarning(std::FILE* f, std::string_view message, const std::source_location& location) -> void {
    if (impl::OnMasterProcess()) {
        PrintWarning(f, message, location);
    }
}

auto MasterPrintWarning(std::ostream& os, std::string_view message, const std::source_location& location) -> void {
    if (impl::OnMasterProcess()) {
        PrintWarning(os, message, location);
    }
}

auto MasterPrintWarning(std::string_view message, const std::source_location& location) -> void {
    if (impl::OnMasterProcess()) {
        PrintWarning(message, location);
    }
}

auto MasterPrintError(std::FILE* f, std::string_view message, const std::source_location& location) -> void {
    if (impl::OnMasterProcess()) {
        PrintError(f, message, location);
    }
}

auto MasterPrintError(std::ostream& os, std::string_view message, const std::source_location& location) -> void {
    if (impl::OnMasterProcess()) {
        PrintError(os, message, location);
    }
}

auto MasterPrintError(std::string_view message, const std::source_location& location) -> void {
    if (impl::OnMasterProcess()) {
        PrintError(message, location);
    }
}

} // namespace Mustard::inline IO
