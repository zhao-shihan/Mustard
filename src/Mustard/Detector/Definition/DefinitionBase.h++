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

#pragma once

#include "Mustard/Utility/NonCopyableBase.h++"
#include "Mustard/Utility/PrettyLog.h++"

#include "G4FieldManager.hh"
#include "G4LogicalVolume.hh"
#include "G4VPhysicalVolume.hh"
#include "G4VSolid.hh"

#include "muc/ceta_string"
#include "muc/ptrvec"
#include "muc/utility"

#include "gsl/gsl"

#include "fmt/core.h"

#include <cassert>
#include <concepts>
#include <filesystem>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <utility>
#include <vector>

class G4Material;
class G4Region;
class G4VSensitiveDetector;

namespace Mustard::Detector::Definition {

class DefinitionBase : public NonCopyableBase {
public:
    virtual ~DefinitionBase() = default;

    auto Topmost() const -> bool { return fMother == nullptr; }
    auto Mother() const -> const DefinitionBase&;

    /// @brief Determines whether we will construct this geometry.
    /// Entities could override this function to control whether this will be constructed.
    /// A typical usage is to get the information of whether to enable this from description in the override function.
    virtual auto Enabled() const -> bool { return true; }

    template<std::derived_from<DefinitionBase> ADefinition>
    auto NewDaughter(bool checkOverlaps) -> ADefinition&;

    auto FindDaughter(std::type_index definition) const -> DefinitionBase*;
    template<std::derived_from<DefinitionBase> ADefinition>
    auto FindDaughter() const -> ADefinition* { return dynamic_cast<ADefinition*>(FindDaughter(typeid(ADefinition))); }

    auto RemoveDaughter(std::type_index definition) -> void;
    template<std::derived_from<DefinitionBase> ADefinition>
    auto RemoveDaughter() -> void { return RemoveDaughter(typeid(ADefinition)); }

    auto FindDescendant(std::type_index definition) const -> DefinitionBase*;
    template<std::derived_from<DefinitionBase> ADefinition>
    auto FindDescendant() const -> ADefinition* { return dynamic_cast<ADefinition*>(FindDescendant(typeid(ADefinition))); }

    auto FindSibling(std::type_index definition) const -> auto { return Mother().FindDaughter(definition); }
    template<std::derived_from<DefinitionBase> ADefinition>
    auto FindSibling() const -> auto { return Mother().FindDaughter<ADefinition>(); }

    auto RegisterMaterial(gsl::not_null<G4Material*> material) const -> void;
    auto RegisterMaterial(std::string_view logicalVolumeName, gsl::not_null<G4Material*> material) const -> void;
    auto RegisterMaterial(gsl::index iLogicalVolume, gsl::not_null<G4Material*> material) const -> void;
    auto RegisterMaterial(std::string_view logicalVolumeName, gsl::index iLogicalVolume, gsl::not_null<G4Material*> material) const -> void;

    auto RegisterRegion(gsl::not_null<G4Region*> region) const -> void;
    auto RegisterRegion(std::string_view logicalVolumeName, gsl::not_null<G4Region*> region) const -> void;
    auto RegisterRegion(gsl::index iLogicalVolume, gsl::not_null<G4Region*> region) const -> void;
    auto RegisterRegion(std::string_view logicalVolumeName, gsl::index iLogicalVolume, gsl::not_null<G4Region*> region) const -> void;

    auto RegisterSD(gsl::not_null<G4VSensitiveDetector*> sd) const -> void;
    auto RegisterSD(std::string_view logicalVolumeName, gsl::not_null<G4VSensitiveDetector*> sd) const -> void;
    auto RegisterSD(gsl::index iLogicalVolume, gsl::not_null<G4VSensitiveDetector*> sd) const -> void;
    auto RegisterSD(std::string_view logicalVolumeName, gsl::index iLogicalVolume, gsl::not_null<G4VSensitiveDetector*> sd) const -> void;

    auto RegisterField(std::unique_ptr<G4FieldManager> fieldManager, bool forceToAllDaughters) -> void;
    auto RegisterField(std::string_view logicalVolumeName, std::unique_ptr<G4FieldManager> fieldManager, bool forceToAllDaughters) -> void;
    auto RegisterField(gsl::index iLogicalVolume, std::unique_ptr<G4FieldManager> fieldManager, bool forceToAllDaughters) -> void;
    auto RegisterField(std::string_view logicalVolumeName, gsl::index iLogicalVolume, std::unique_ptr<G4FieldManager> fieldManager, bool forceToAllDaughters) -> void;

    auto Export(std::filesystem::path gdmlFile, gsl::index iPhysicalVolume = 0) const -> void;
    auto Export(std::filesystem::path gdmlFile, std::string_view physicalVolumeName, gsl::index iPhysicalVolume = 0) const -> void;
    auto ParallelExport(std::filesystem::path gdmlFile, gsl::index iPhysicalVolume = 0) const -> std::filesystem::path;
    auto ParallelExport(std::filesystem::path gdmlFile, std::string_view physicalVolumeName, gsl::index iPhysicalVolume = 0) const -> std::filesystem::path;

    auto LogicalVolumes() const -> const std::vector<G4LogicalVolume*>&;
    auto LogicalVolumes(std::string_view name) const -> const std::vector<G4LogicalVolume*>&;
    auto LogicalVolume(gsl::index i = 0) const -> G4LogicalVolume*;
    auto LogicalVolume(std::string_view name, gsl::index i = 0) const -> G4LogicalVolume*;

    auto PhysicalVolumes() const -> const std::vector<G4VPhysicalVolume*>&;
    auto PhysicalVolumes(std::string_view name) const -> const std::vector<G4VPhysicalVolume*>&;
    auto PhysicalVolume(gsl::index i = 0) const -> G4VPhysicalVolume*;
    auto PhysicalVolume(std::string_view name, gsl::index i = 0) const -> G4VPhysicalVolume*;

protected:
    // Make a G4Solid and keep it (for deleting when geometry deconstructs).
    template<std::derived_from<G4VSolid> ASolid>
    auto Make(auto&&... args) -> gsl::not_null<ASolid*>;
    // Make a G4LogicalVolume and keep it for further access. Will be deleted when geometry deconstructs.
    template<std::derived_from<G4LogicalVolume> ALogical>
    auto Make(auto&&... args) -> gsl::not_null<ALogical*>;
    // Make a G4PhysicalVolume and keep it for further access. Will be deleted when geometry deconstructs.
    template<std::derived_from<G4VPhysicalVolume> APhysical>
    auto Make(auto&&... args) -> gsl::not_null<APhysical*>;

private:
    virtual auto Construct(bool checkOverlaps) -> void = 0;

    template<muc::ceta_string AMode>
        requires(AMode == "warning" or AMode == "quiet")
    auto Ready() const -> bool;

private:
    const DefinitionBase* fMother{};

    muc::unique_ptrvec<G4VSolid> fSolidStore;
    muc::unique_ptrvec<G4LogicalVolume> fLogicalVolumeStore;
    muc::unique_ptrvec<G4VPhysicalVolume> fPhysicalVolumeStore;

    std::unordered_map<std::string, std::vector<G4LogicalVolume*>> fLogicalVolumes;
    const std::vector<G4LogicalVolume*>* fFirstLogicalVolumes{};
    std::unordered_map<std::string, std::vector<G4VPhysicalVolume*>> fPhysicalVolumes;
    const std::vector<G4VPhysicalVolume*>* fFirstPhysicalVolumes{};

    muc::unique_ptrvec<G4FieldManager> fFieldStore;

    std::unordered_map<std::type_index, std::unique_ptr<DefinitionBase>> fDaughters;
};

} // namespace Mustard::Detector::Definition

#include "Mustard/Detector/Definition/DefinitionBase.inl"
