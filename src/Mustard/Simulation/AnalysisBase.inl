namespace Mustard::Simulation {

template<typename ADerived, muc::ceta_string AAppName>
AnalysisBase<ADerived, AAppName>::AnalysisBase(ADerived* self) :
    Env::Memory::PassiveSingleton<ADerived>{self},
    fFilePath{fmt::format("{}_untitled", AAppName.sv())},
    fFileMode{"NEW"},
    fLastUsedFullFilePath{},
    fFile{},
    fMessengerRegister{self} {
    static_assert(std::derived_from<ADerived, AnalysisBase<ADerived, AAppName>>);
}

template<typename ADerived, muc::ceta_string AAppName>
auto AnalysisBase<ADerived, AAppName>::RunBeginAction(int runID) -> void {
    // open ROOT file
    const auto filePathChanged{fFilePath != fLastUsedFullFilePath};
    fFile.emplace(fFilePath, filePathChanged ? fFileMode : "UPDATE");
    fLastUsedFullFilePath = fFilePath;
    // save geometry
    if (filePathChanged and mplr::comm_world().rank() == 0) {
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
    fFile.reset();
}

} // namespace Mustard::Simulation
