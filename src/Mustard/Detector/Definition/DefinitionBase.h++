#pragma once

#include "Mustard/Utility/NonMoveableBase.h++"

#include "G4FieldManager.hh"
#include "G4LogicalVolume.hh"
#include "G4VPhysicalVolume.hh"
#include "G4VSolid.hh"

#include "gsl/gsl"

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

class DefinitionBase : public NonMoveableBase {
public:
    virtual ~DefinitionBase() = default;

    auto Topmost() const -> bool { return fMother == nullptr; }
    auto Mother() const -> const auto& { return *fMother; }

    /// @brief Determines whether we will construct this geometry.
    /// Entities could override this function to control whether this will be constructed.
    /// A typical usage is to get the information of whether to enable this from description in the override function.
    virtual auto Enabled() const -> bool { return true; }

    template<std::derived_from<DefinitionBase> ADefinition>
    auto NewDaughter(bool checkOverlaps) -> ADefinition&;
    template<std::derived_from<DefinitionBase> ADefinition>
    auto FindDaughter() const -> ADefinition*;
    template<std::derived_from<DefinitionBase> ADefinition>
    auto RemoveDaughter() -> bool { return fDaughters.erase(typeid(ADefinition)) > 0; }
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

    auto Export(const std::filesystem::path& gdmlFile, gsl::index iPhysicalVolume = 0) const -> void;
    auto Export(const std::filesystem::path& gdmlFile, std::string_view physicalVolumeName, gsl::index iPhysicalVolume = 0) const -> void;

    auto LogicalVolumes() const -> const std::vector<G4LogicalVolume*>&;
    auto LogicalVolumes(std::string_view name) const -> const auto& { return fLogicalVolumes.at(std::string{name}); }
    auto LogicalVolume(gsl::index i = 0) const -> auto { return LogicalVolumes().at(i); }
    auto LogicalVolume(std::string_view name, gsl::index i = 0) const -> auto { return LogicalVolumes(name).at(i); }

    auto PhysicalVolumes() const -> const std::vector<G4VPhysicalVolume*>&;
    auto PhysicalVolumes(std::string_view name) const -> const auto& { return fPhysicalVolumes.at(std::string{name}); }
    auto PhysicalVolume(gsl::index i = 0) const -> auto { return PhysicalVolumes().at(i); }
    auto PhysicalVolume(std::string_view name, gsl::index i = 0) const -> auto { return PhysicalVolumes(name).at(i); }

protected:
    // Make a G4Solid and keep it (for deleting when geometry deconstructs).
    template<std::derived_from<G4VSolid> ASolid>
    auto Make(auto&&... args) -> gsl::not_null<ASolid*>;
    // Make a G4LogicalVolume and keep it for futher access. Will be deleted when geometry deconstructs.
    template<std::derived_from<G4LogicalVolume> ALogical>
    auto Make(auto&&... args) -> gsl::not_null<ALogical*>;
    // Make a G4PhysicalVolume and keep it for futher access. Will be deleted when geometry deconstructs.
    template<std::derived_from<G4VPhysicalVolume> APhysical>
    auto Make(auto&&... args) -> gsl::not_null<APhysical*>;

private:
    virtual auto Construct(bool checkOverlaps) -> void = 0;

    auto Ready() const -> bool { return fPhysicalVolumes.size() > 0; }

private:
    const DefinitionBase* fMother{};

    std::vector<std::unique_ptr<G4VSolid>> fSolidStore;
    std::vector<std::unique_ptr<G4LogicalVolume>> fLogicalVolumeStore;
    std::vector<std::unique_ptr<G4VPhysicalVolume>> fPhysicalVolumeStore;

    std::unordered_map<std::string, std::vector<G4LogicalVolume*>> fLogicalVolumes;
    const std::vector<G4LogicalVolume*>* fFirstLogicalVolumes{};
    std::unordered_map<std::string, std::vector<G4VPhysicalVolume*>> fPhysicalVolumes;
    const std::vector<G4VPhysicalVolume*>* fFirstPhysicalVolumes{};

    std::vector<std::unique_ptr<G4FieldManager>> fFieldStore;

    std::unordered_map<std::type_index, std::unique_ptr<DefinitionBase>> fDaughters;
};

} // namespace Mustard::Detector::Definition

#include "Mustard/Detector/Definition/DefinitionBase.inl"
