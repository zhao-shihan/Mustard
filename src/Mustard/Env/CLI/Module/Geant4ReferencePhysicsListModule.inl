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

namespace Mustard::Env::CLI::inline Module {

template<muc::ceta_string ADefault>
Geant4ReferencePhysicsListModule<ADefault>::Geant4ReferencePhysicsListModule(argparse::ArgumentParser& argParser) :
    ModuleBase{argParser} {
    ArgParser()
        .add_argument("--physics-list")
        .default_value(ADefault.sv())
        .help("Set reference physics list use in the simulation.");
}

template<muc::ceta_string ADefault>
auto Geant4ReferencePhysicsListModule<ADefault>::PhysicsList() const -> G4VModularPhysicsList* {
    if (fReferencePhysicsList) {
        throw std::logic_error{fmt::format("Geant4ReferencePhysicsListModule<\"{}\">::PhysicsList() called twice", ADefault.sv())};
    }

    const auto physicsList{ArgParser().get("--physics-list")};
    G4PhysListFactory physicsListFactory{muc::to_underlying(BasicEnv::Instance().VerboseLevel())};

    if (not physicsListFactory.IsReferencePhysList(physicsList)) {
        throw std::runtime_error{fmt::format("{} is not a reference physics list")};
    }

    fReferencePhysicsList = physicsListFactory.GetReferencePhysList(physicsList);
    return fReferencePhysicsList.get();
}

} // namespace Mustard::Env::CLI::inline Module
