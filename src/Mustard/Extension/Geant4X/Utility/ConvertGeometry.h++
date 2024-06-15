#pragma once

#include <filesystem>
#include <memory>
#include <string>

class TMacro;

class G4LogicalVolume;

namespace Mustard::inline Extension::Geant4X::inline Utility {

auto ConvertGeometryToGDMLText(const G4LogicalVolume* g4Geom = {}) -> std::string;
auto ConvertGeometryToTMacro(const std::string& name, const std::filesystem::path& output, const G4LogicalVolume* g4Geom = {}) -> std::unique_ptr<TMacro>;

} // namespace Mustard::inline Extension::Geant4X::inline Utility
