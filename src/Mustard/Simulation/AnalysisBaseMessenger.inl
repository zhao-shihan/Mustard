namespace Mustard::Simulation {

template<typename Analysis>
AnalysisBaseMessenger<Analysis>::AnalysisBaseMessenger() :
    Geant4X::SingletonMessenger<AnalysisBaseMessenger<Analysis>, Analysis>{},
    fDirectory{},
    fFilePath{},
    fFileMode{} {

    fDirectory = std::make_unique<G4UIdirectory>("/Mustard/Analysis/");
    fDirectory->SetGuidance("Simulation analysis controller.");

    fFilePath = std::make_unique<G4UIcmdWithAString>("/Mustard/Analysis/FilePath", this);
    fFilePath->SetGuidance("Set file path.");
    fFilePath->SetParameterName("path", false);
    fFilePath->AvailableForStates(G4State_Idle);

    fFileMode = std::make_unique<G4UIcmdWithAString>("/Mustard/Analysis/FileMode", this);
    fFileMode->SetGuidance("Set mode (NEW, RECREATE, or UPDATE) for opening ROOT file(s).");
    fFileMode->SetParameterName("mode", false);
    fFileMode->AvailableForStates(G4State_Idle);
}

template<typename Analysis>
auto AnalysisBaseMessenger<Analysis>::SetNewValue(G4UIcommand* command, G4String value) -> void {
    if (command == fFilePath.get()) {
        this->template Deliver<Analysis>([&](auto&& r) {
            r.FilePath(std::string_view(value));
        });
    } else if (command == fFileMode.get()) {
        this->template Deliver<Analysis>([&](auto&& r) {
            r.FileMode(value);
        });
    }
}

} // namespace Mustard::Simulation
