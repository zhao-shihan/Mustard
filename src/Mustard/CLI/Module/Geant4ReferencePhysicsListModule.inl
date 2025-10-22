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

namespace Mustard::CLI::inline Module {

template<muc::ceta_string ADefault>
Geant4ReferencePhysicsListModule<ADefault>::Geant4ReferencePhysicsListModule(gsl::not_null<CLI<>*> cli) :
    ModuleBase{cli},
    fReferencePhysicsList{} {
    TheCLI()
        ->add_argument("--physics-list")
        .default_value(ADefault.s())
        .required()
        .help("Set reference physics list use in the simulation.");
}

template<muc::ceta_string ADefault>
auto Geant4ReferencePhysicsListModule<ADefault>::PhysicsList() -> G4VModularPhysicsList* {
    if (fReferencePhysicsList) {
        return fReferencePhysicsList;
    }

    const auto physicsList{TheCLI()->get("--physics-list")};
    G4PhysListFactory physicsListFactory{muc::to_underlying(Env::BasicEnv::Instance().VerboseLevel())};

    if (not physicsListFactory.IsReferencePhysList(physicsList)) {
        Throw<std::runtime_error>(fmt::format("{} is not a reference physics list", physicsList));
    }

    fReferencePhysicsList = physicsListFactory.GetReferencePhysList(physicsList);
    return fReferencePhysicsList;
}

} // namespace Mustard::CLI::inline Module
