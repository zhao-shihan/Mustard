// -*- C++ -*-
//
// Copyright (C) 2020-2026  Shihan Zhao and contributors
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

namespace Mustard::Simulation {

template<typename ADerived, muc::ceta_string AAppName>
AnalysisBase<ADerived, AAppName>::AnalysisBase(ADerived* self) :
    Env::PassiveSingleton<ADerived>{self},
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
