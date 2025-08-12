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

#include "Mustard/Geant4X/Utility/ConvertGeometry.h++"
#include "Mustard/IO/CreateTemporaryFile.h++"
#include "Mustard/IO/PrettyLog.h++"

#include "TMacro.h"

#include "G4GDMLParser.hh"
#include "G4LogicalVolume.hh"

#include "fmt/format.h"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <system_error>

namespace Mustard::Geant4X::inline Utility {

auto ConvertGeometryToGDMLText(const G4LogicalVolume* g4Geom) -> std::string {
    const auto tempGDMLPath{CreateTemporaryFile("g4geom", ".gdml")};
    {
        G4GDMLParser gdml;
        gdml.SetAddPointerToName(true);
        gdml.SetOutputFileOverwrite(true);
        const auto g4coutBuf{G4cout.rdbuf(nullptr)};
        gdml.Write(tempGDMLPath.generic_string(), g4Geom);
        G4cout.rdbuf(g4coutBuf);
    }
    std::ostringstream tempText;
    {
        std::ifstream tempGDML{tempGDMLPath};
        if (tempGDML.fail()) {
            Throw<std::runtime_error>("Error opening temp gdml file");
        }
        tempText << tempGDML.rdbuf();
    }
    std::error_code muteRemoveError;
    std::filesystem::remove(tempGDMLPath, muteRemoveError);
    return tempText.str();
}

auto ConvertGeometryToTMacro(const std::string& name, const std::filesystem::path& output, const G4LogicalVolume* g4Geom) -> std::unique_ptr<TMacro> {
    const auto tempMacroPath{CreateTemporaryFile(name, ".C")};
    {
        const auto tempMacroFile{std::fopen(tempMacroPath.generic_string().c_str(), "w")};
        if (tempMacroFile == nullptr) {
            Throw<std::runtime_error>("Error opening temp macro file");
        }
        fmt::println(tempMacroFile, R"macro(
#include <fstream>
#include <iostream>

auto {0}() -> void {{
    std::ofstream gdmlFile{{"{1}"}};
    if (gdmlFile.fail()) {{
        std::cerr << "\nerror opening {1}" << std::endl;
        return;
    }}
    gdmlFile << R"{0}({2}){0}""\n";
    std::cout << "\nGDML has been saved to {1}" << std::endl;
}}
)macro",
                     name, output.generic_string(), ConvertGeometryToGDMLText(g4Geom));
        std::fclose(tempMacroFile);
    }
    auto macro{std::make_unique<TMacro>(name.c_str(), "Generate GDML file")};
    {
        const auto lines{macro->ReadFile(tempMacroPath.generic_string().c_str())};
        if (lines == 0) {
            Throw<std::runtime_error>("Error opening temp macro file");
        }
    }
    std::error_code muteRemoveError;
    std::filesystem::remove(tempMacroPath, muteRemoveError);
    return macro;
}

} // namespace Mustard::Geant4X::inline Utility
