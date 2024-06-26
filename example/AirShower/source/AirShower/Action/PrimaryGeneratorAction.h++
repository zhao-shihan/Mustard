#pragma once

#include "AirShower/Data/PrimaryVertex.h++"
#include "AirShower/Messenger/AnalysisMessenger.h++"

#include "Mustard/Data/Tuple.h++"
#include "Mustard/Env/Memory/PassiveSingleton.h++"
#include "Mustard/Extension/Geant4X/Generator/GeneralParticleSourceX.h++"

#include "G4VUserPrimaryGeneratorAction.hh"

#include "muc/ptr_vector"

#include <memory>
#include <vector>

namespace AirShower::inline Action {

class PrimaryGeneratorAction final : public Mustard::Env::Memory::PassiveSingleton<PrimaryGeneratorAction>,
                                     public G4VUserPrimaryGeneratorAction {
public:
    PrimaryGeneratorAction();

    auto SavePrimaryVertexData() const -> auto { return fSavePrimaryVertexData; }
    auto SavePrimaryVertexData(bool val) -> void { fSavePrimaryVertexData = val; }

    auto GeneratePrimaries(G4Event* event) -> void override;

private:
    auto UpdatePrimaryVertexData(const G4Event& event) -> void;

private:
    Mustard::Geant4X::GeneralParticleSourceX fGPSX;

    bool fSavePrimaryVertexData;
    muc::unique_ptr_vector<Mustard::Data::Tuple<Data::PrimaryVertex>> fPrimaryVertexData;

    AnalysisMessenger::Register<PrimaryGeneratorAction> fMessengerRegister;
};

} // namespace AirShower::inline Action
