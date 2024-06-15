namespace Mustard::inline Extension::Geant4X::inline Physics {

template<TargetForMuoniumPhysics ATarget>
MuoniumPhysicsMessenger<ATarget>::MuoniumPhysicsMessenger() :
    Geant4X::SingletonMessenger<MuoniumPhysicsMessenger<ATarget>,
                                MuoniumFormation<ATarget>,
                                MuoniumTransport<ATarget>>{},
    fMuoniumPhysicsDirectory{},
    fFormationProcessDirectory{},
    fConversionProbability{},
    fTransportProcessDirectory{},
    fManipulateAllSteps{} {

    fMuoniumPhysicsDirectory = std::make_unique<G4UIdirectory>("/Mustard/Physics/MuoniumPhysics/");
    fMuoniumPhysicsDirectory->SetGuidance("Physics of muonium and anti-muonium.");

    fFormationProcessDirectory = std::make_unique<G4UIdirectory>("/Mustard/Physics/MuoniumPhysics/Formation/");
    fFormationProcessDirectory->SetGuidance("Muonium formation and transition process.");

    fConversionProbability = std::make_unique<G4UIcmdWithADouble>("/Mustard/Physics/MuoniumPhysics/Formation/ConversionProbability", this);
    fConversionProbability->SetGuidance("Set integrated probability of muonium to anti-muonium conversion.");
    fConversionProbability->SetParameterName("P", false);
    fConversionProbability->SetRange("0 <= P && P <= 1");
    fConversionProbability->AvailableForStates(G4State_Idle);

    fTransportProcessDirectory = std::make_unique<G4UIdirectory>("/Mustard/Physics/MuoniumPhysics/Transport/");
    fTransportProcessDirectory->SetGuidance("The transport process of thermal muonium in the target.");

    fManipulateAllSteps = std::make_unique<G4UIcmdWithABool>("/Mustard/Physics/MuoniumPhysics/Transport/ManipulateAllSteps", this),
    fManipulateAllSteps->SetGuidance("Set whether show each step of thermal random flight of muonium in the target or not.\n"
                                     "Warning: can be time consuming if set to true.");
    fManipulateAllSteps->SetParameterName("bool", false);
    fManipulateAllSteps->AvailableForStates(G4State_Idle);
}

template<TargetForMuoniumPhysics ATarget>
auto MuoniumPhysicsMessenger<ATarget>::SetNewValue(G4UIcommand* command, G4String value) -> void {
    if (command == fConversionProbability.get()) {
        this->template Deliver<MuoniumFormation<ATarget>>([&](auto&& r) {
            r.ConversionProbability(fConversionProbability->GetNewDoubleValue(value));
        });
    } else if (command == fManipulateAllSteps.get()) {
        this->template Deliver<MuoniumTransport<ATarget>>([&](auto&& r) {
            r.ManipulateAllSteps(fManipulateAllSteps->GetNewBoolValue(value));
        });
    }
}

} // namespace Mustard::inline Extension::Geant4X::inline Physics
