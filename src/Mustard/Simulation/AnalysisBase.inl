namespace Mustard::Simulation {

template<typename ADerived, muc::ceta_string AAppName>
AnalysisBase<ADerived, AAppName>::AnalysisBase() :
    Env::Memory::PassiveSingleton<ADerived>{},
    fFilePath{fmt::format("{}_untitled", AAppName.sv())},
    fFileMode{"NEW"},
    fLastUsedFullFilePath{},
    fFile{},
    fMessengerRegister{static_cast<ADerived*>(this)} {
    static_assert(std::derived_from<ADerived, AnalysisBase<ADerived, AAppName>>);
}

template<typename ADerived, muc::ceta_string AAppName>
auto AnalysisBase<ADerived, AAppName>::RunBeginAction(int runID) -> void {
    // open ROOT file
    auto fullFilePath{Mustard::MPIX::ParallelizePath(fFilePath).replace_extension(".root").generic_string()};
    const auto filePathChanged{fullFilePath != fLastUsedFullFilePath};
    fFile = TFile::Open(fullFilePath.c_str(), filePathChanged ? fFileMode.c_str() : "UPDATE",
                        "", ROOT::RCompressionSetting::EDefaults::kUseGeneralPurpose);
    if (fFile == nullptr) {
        throw std::runtime_error{fmt::format("Mustard::AnalysisBase::RunBeginAction: Cannot open file '{}' with mode '{}'",
                                             fullFilePath, fFileMode)};
    }
    fLastUsedFullFilePath = std::move(fullFilePath);
    // save geometry
    if (filePathChanged and Mustard::Env::MPIEnv::Instance().OnCommWorldMaster()) {
        Mustard::Geant4X::ConvertGeometryToTMacro(fmt::format("{}_gdml", AAppName.sv()),
                                                  fmt::format("{}.gdml", AAppName.sv()))
            ->Write();
    }
    // initialize outputs
    RunBeginUserAction(runID);
}

template<typename ADerived, muc::ceta_string AAppName>
auto AnalysisBase<ADerived, AAppName>::EventEndAction() -> void {
    EventEndUserAction();
}

template<typename ADerived, muc::ceta_string AAppName>
auto AnalysisBase<ADerived, AAppName>::RunEndAction(int runID) -> void {
    RunEndUserAction(runID);
    // close file
    delete fFile;
}

} // namespace Mustard::Simulation
