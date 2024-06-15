#pragma once

#include "Mustard/Extension/Geant4X/Generator/GeneralParticleSourceXMessenger.h++"

#include "G4GeneralParticleSource.hh"

namespace Mustard::inline Extension::Geant4X::inline Generator {

class GeneralParticleSourceX : public G4GeneralParticleSource {
public:
    GeneralParticleSourceX();

    auto NVertex() const -> auto { return fNVertex; }
    auto PulseWidth() const -> auto { return fPulseWidth; }

    auto NVertex(int n) -> void { fNVertex = std::max(0, n); }
    auto PulseWidth(double val) -> void { fPulseWidth = val; }

    auto GeneratePrimaryVertex(G4Event*) -> void override;

private:
    int fNVertex;
    double fPulseWidth;

    GeneralParticleSourceXMessenger::Register<GeneralParticleSourceX> fMessengerRegister;
};

} // namespace Mustard::inline Extension::Geant4X::inline Generator
