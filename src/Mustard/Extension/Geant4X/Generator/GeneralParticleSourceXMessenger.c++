#include "Mustard/Extension/Geant4X/Generator/GeneralParticleSourceX.h++"
#include "Mustard/Extension/Geant4X/Generator/GeneralParticleSourceXMessenger.h++"

#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIcmdWithAnInteger.hh"

namespace Mustard::inline Extension::Geant4X::inline Generator {

GeneralParticleSourceXMessenger::GeneralParticleSourceXMessenger() :
    SingletonMessenger{},
    fDirectory{},
    fNVertex{},
    fPulseWidth{} {

    fDirectory = std::make_unique<G4UIdirectory>("/GPSX/");
    fDirectory->SetGuidance("General particle source extension.");

    fNVertex = std::make_unique<G4UIcmdWithAnInteger>("/GPSX/NVertex", this);
    fNVertex->SetGuidance("Set number of vertices to generate in an event.");
    fNVertex->SetParameterName("N", false);
    fNVertex->SetRange("N >= 0");
    fNVertex->AvailableForStates(G4State_Idle);

    fPulseWidth = std::make_unique<G4UIcmdWithADoubleAndUnit>("/GPSX/PulseWidth", this);
    fPulseWidth->SetGuidance("Set pulse width of vertex time.");
    fPulseWidth->SetParameterName("T", false);
    fPulseWidth->SetUnitCategory("Time");
    fPulseWidth->AvailableForStates(G4State_Idle);
}

GeneralParticleSourceXMessenger::~GeneralParticleSourceXMessenger() = default;

auto GeneralParticleSourceXMessenger::SetNewValue(G4UIcommand* command, G4String value) -> void {
    if (command == fNVertex.get()) {
        Deliver<GeneralParticleSourceX>([&](auto&& r) {
            r.NVertex(fNVertex->GetNewIntValue(value));
        });
    } else if (command == fPulseWidth.get()) {
        Deliver<GeneralParticleSourceX>([&](auto&& r) {
            r.PulseWidth(fPulseWidth->GetNewDoubleValue(value));
        });
    }
}

} // namespace Mustard::inline Extension::Geant4X::inline Generator
