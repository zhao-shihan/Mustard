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

namespace Mustard::Detector::Definition {

template<std::derived_from<DefinitionBase> ADefinition>
auto DefinitionBase::NewDaughter(bool checkOverlaps) -> ADefinition& {
    if (typeid(ADefinition) == typeid(*this)) {
        throw std::logic_error{"Mustard::Detector::Definition::DefinitionBase::AddDaughter: "
                               "Trying to add the same geometry to itself as a daughter"};
    }

    const auto [iterator, emplaced]{fDaughters.try_emplace(typeid(ADefinition), std::make_unique_for_overwrite<ADefinition>())};
    if (not emplaced) {
        throw std::logic_error{"Mustard::Detector::Definition::DefinitionBase::AddDaughter: "
                               "Trying to add the same geometry to itself as a daughter"};
    }
    const auto& daughter{iterator->second};
    daughter->fMother = this;

    if (Topmost() and Enabled()) {
        Construct(checkOverlaps);
    }
    if (Ready() and daughter->Enabled()) {
        daughter->Construct(checkOverlaps);
    }

    return static_cast<ADefinition&>(*daughter);
}

template<std::derived_from<DefinitionBase> ADefinition>
auto DefinitionBase::FindDaughter() const -> ADefinition* {
    if (const auto existedDaughter{fDaughters.find(typeid(ADefinition))};
        existedDaughter != fDaughters.cend()) {
        return static_cast<ADefinition*>(existedDaughter->second.get());
    } else {
        return {};
    }
}

template<std::derived_from<G4VSolid> ASolid>
auto DefinitionBase::Make(auto&&... args) -> gsl::not_null<ASolid*> {
    return static_cast<ASolid*>(
        fSolidStore
            .emplace_back(
                std::make_unique<ASolid>(std::forward<decltype(args)>(args)...))
            .get());
}

template<std::derived_from<G4LogicalVolume> ALogical>
auto DefinitionBase::Make(auto&&... args) -> gsl::not_null<ALogical*> {
    const auto logic{
        fLogicalVolumeStore
            .emplace_back(
                std::make_unique<ALogical>(std::forward<decltype(args)>(args)...))
            .get()};
    fLogicalVolumes[logic->GetName()].emplace_back(logic);
    if (fFirstLogicalVolumes == nullptr) {
        assert(fLogicalVolumeStore.size() == 1);
        assert(fLogicalVolumes.size() == 1);
        fFirstLogicalVolumes = &fLogicalVolumes.cbegin()->second;
        assert(fFirstLogicalVolumes->size() == 1);
    }
    return static_cast<ALogical*>(logic);
}

template<std::derived_from<G4VPhysicalVolume> APhysical>
auto DefinitionBase::Make(auto&&... args) -> gsl::not_null<APhysical*> {
    const auto physics{
        fPhysicalVolumeStore
            .emplace_back(
                std::make_unique<APhysical>(std::forward<decltype(args)>(args)...))
            .get()};
    fPhysicalVolumes[physics->GetName()].emplace_back(physics);
    if (fFirstPhysicalVolumes == nullptr) {
        assert(fPhysicalVolumeStore.size() == 1);
        assert(fPhysicalVolumes.size() == 1);
        fFirstPhysicalVolumes = &fPhysicalVolumes.cbegin()->second;
        assert(fFirstPhysicalVolumes->size() == 1);
    }
    return static_cast<APhysical*>(physics);
}

} // namespace Mustard::Detector::Definition
