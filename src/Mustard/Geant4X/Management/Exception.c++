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

#include "Mustard/Geant4X/Management/Exception.h++"
#include "Mustard/IO/PrettyLog.h++"

#include "mplr/mplr.hpp"

#include "fmt/format.h"

#include <functional>
#include <string>

namespace Mustard::Geant4X::inline Management {

auto Exception(G4ExceptionSeverity severity,
               const std::string& message,
               const std::source_location& loc,
               const std::string& code) -> void {
    const auto& actualCode{
        code.empty() ? fmt::format("0x{:x}", std::hash<std::string>{}(std::format(
                                                 "{}:{}:{}", loc.file_name(), loc.line(), loc.column()))) :
                       code};
    switch (severity) {
    case FatalException:
    case FatalErrorInArgument:
    case RunMustBeAborted:
    case EventMustBeAborted:
        PrintError(message, loc);
        break;
    case JustWarning:
        PrintWarning(message, loc);
        break;
    case IgnoreTheIssue:
        break;
    }
    G4Exception(loc.function_name(), actualCode.c_str(), severity, message.c_str());
}

auto MasterException(G4ExceptionSeverity severity,
                     const std::string& message,
                     const std::source_location& loc,
                     const std::string& code) -> void {
    if (not mplr::available() or mplr::comm_world().rank() == 0) {
        Exception(severity, message, loc, code);
    }
}

} // namespace Mustard::Geant4X::inline Management
