namespace Mustard::inline Extension::Geant4X::inline Interface {

template<typename ADerived, std::derived_from<G4VUserDetectorConstruction> ADetectorConstruction, muc::ceta_string AAppName>
DetectorMessenger<ADerived, ADetectorConstruction, AAppName>::DetectorMessenger() :
    Geant4X::SingletonMessenger<ADerived>{},
    fDirectory{},
    fImportDescription{},
    fExportDescription{},
    fIxportDescription{} {
    static_assert(std::derived_from<ADerived, DetectorMessenger<ADerived, ADetectorConstruction, AAppName>>);

    fDirectory = std::make_unique<G4UIdirectory>("/Mustard/Detector/");

    fImportDescription = std::make_unique<G4UIcmdWithAString>("/Mustard/Detector/Description/Import", this);
    fImportDescription->SetGuidance("Import geometry descriptions required by this program from a yaml file.");
    fImportDescription->SetParameterName("yaml", false);
    fImportDescription->AvailableForStates(G4State_PreInit);

    fExportDescription = std::make_unique<G4UIcmdWithAString>("/Mustard/Detector/Description/Export", this);
    fExportDescription->SetGuidance("Export geometry descriptions used by this program to a yaml file.");
    fExportDescription->SetParameterName("yaml", false);
    fExportDescription->AvailableForStates(G4State_PreInit, G4State_Idle);

    fIxportDescription = std::make_unique<G4UIcmdWithAString>("/Mustard/Detector/Description/Ixport", this);
    fIxportDescription->SetGuidance("Export, Import, then export geometry descriptions used by this program. "
                                    "Exported files have '.prev' (previous) or '.curr' (current) suffix, respectively.");
    fIxportDescription->SetParameterName("yaml", false);
    fIxportDescription->AvailableForStates(G4State_PreInit);
}

template<typename ADerived, std::derived_from<G4VUserDetectorConstruction> ADetectorConstruction, muc::ceta_string AAppName>
auto DetectorMessenger<ADerived, ADetectorConstruction, AAppName>::SetNewValue(G4UIcommand* command, G4String value) -> void {
    using DescriptionInUse = ADetectorConstruction::DescriptionInUse;
    using Detector::Description::DescriptionIO;
    const auto annotation{[] {
        if constexpr (AAppName) {
            return fmt::format("{}: geometry description", AAppName.sv());
        } else {
            return std::string{};
        }
    }()};
    if (command == fImportDescription.get()) {
        DescriptionIO::Import<DescriptionInUse>(std::string_view{value});
    } else if (command == fExportDescription.get()) {
        DescriptionIO::Export<DescriptionInUse>(std::string_view{value}, annotation);
    } else if (command == fIxportDescription.get()) {
        DescriptionIO::Ixport<DescriptionInUse>(std::string_view{value}, annotation);
    }
}

} // namespace Mustard::inline Extension::Geant4X::inline Interface
