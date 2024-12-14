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

#include "Mustard/Utility/CreateTemporaryFile.h++"
#include "Mustard/Utility/MakeTextTMacro.h++"
#include "Mustard/Utility/PrettyLog.h++"

#include "TMacro.h"

#include "fmt/format.h"

#include <cstdio>
#include <filesystem>
#include <stdexcept>
#include <system_error>

namespace Mustard::inline Utility {

auto MakeTextTMacro(std::string_view text, const std::string& name, const std::string& title) -> std::unique_ptr<TMacro> {
    const auto tempMacroPath{CreateTemporaryFile(name, ".C")};
    {
        const auto tempMacroFile{std::fopen(tempMacroPath.generic_string().c_str(), "w")};
        if (tempMacroFile == nullptr) { Throw<std::runtime_error>("Error opening temp macro file"); }
        fmt::println(tempMacroFile, R"macro(
#include <iostream>

auto {0}() -> void {{
    std::cout << "\n" R"{0}({1}){0}" << std::endl;
}}
)macro",
                     name, text);
        std::fclose(tempMacroFile);
    }
    auto macro{std::make_unique<TMacro>(name.c_str(), title.c_str())};
    {
        const auto lines{macro->ReadFile(tempMacroPath.generic_string().c_str())};
        if (lines == 0) { Throw<std::runtime_error>("Error opening temp macro file"); }
    }
    std::error_code muteRemoveError;
    std::filesystem::remove(tempMacroPath, muteRemoveError);
    return macro;
}

} // namespace Mustard::inline Utility
