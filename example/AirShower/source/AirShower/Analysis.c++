#include "AirShower/Action/PrimaryGeneratorAction.h++"
#include "AirShower/Action/TrackingAction.h++"
#include "AirShower/Analysis.h++"

#include "Mustard/Env/MPIEnv.h++"
#include "Mustard/Extension/Geant4X/Utility/ConvertGeometry.h++"
#include "Mustard/Extension/MPIX/ParallelizePath.h++"
#include "Mustard/Utility/PrettyLog.h++"

#include "TFile.h"
#include "TMacro.h"

#include "fmt/format.h"

#include <optional>
#include <stdexcept>

namespace AirShower {

Analysis::Analysis() :
    PassiveSingleton{},
    fFilePath{"AirShower_untitled"},
    fFileMode{"NEW"},
    fLastUsedFullFilePath{},
    fFile{},
    fPrimaryVertexOutput{},
    fDecayVertexOutput{},
    fEarthHitOutput{},
    fPrimaryVertex{},
    fDecayVertex{},
    fEarthHit{},
    fMessengerRegister{this} {}

auto Analysis::RunBegin(int runID) -> void {
    // open ROOT file
    auto fullFilePath{Mustard::MPIX::ParallelizePath(fFilePath).replace_extension(".root").generic_string()};
    const auto filePathChanged{fullFilePath != fLastUsedFullFilePath};
    fFile = TFile::Open(fullFilePath.c_str(), filePathChanged ? fFileMode.c_str() : "UPDATE",
                        "", ROOT::RCompressionSetting::EDefaults::kUseGeneralPurpose);
    if (fFile == nullptr) {
        Mustard::Throw<std::runtime_error>(fmt::format("Cannot open file '{}' with mode '{}'", fullFilePath, fFileMode));
    }
    fLastUsedFullFilePath = std::move(fullFilePath);
    // save geometry
    if (filePathChanged and Mustard::Env::MPIEnv::Instance().OnCommWorldMaster()) {
        Mustard::Geant4X::ConvertGeometryToTMacro("AirShower_gdml", "AirShower.gdml")->Write();
    }
    // initialize outputs
    if (PrimaryGeneratorAction::Instance().SavePrimaryVertexData()) { fPrimaryVertexOutput.emplace(fmt::format("G4Run{}/SimPrimaryVertex", runID)); }
    if (TrackingAction::Instance().SaveDecayVertexData()) { fDecayVertexOutput.emplace(fmt::format("G4Run{}/SimDecayVertex", runID)); }
    fEarthHitOutput.emplace(fmt::format("G4Run{}/EarthHit", runID));
}

auto Analysis::EventEnd() -> void {
    if (fPrimaryVertex and fPrimaryVertexOutput) { fPrimaryVertexOutput->Fill(*fPrimaryVertex); }
    if (fDecayVertex and fDecayVertexOutput) { fDecayVertexOutput->Fill(*fDecayVertex); }
    if (fEarthHit) { fEarthHitOutput->Fill(*fEarthHit); }
    fPrimaryVertex = {};
    fDecayVertex = {};
    fEarthHit = {};
}

auto Analysis::RunEnd(Option_t* option) -> void {
    // write data
    if (fPrimaryVertexOutput) { fPrimaryVertexOutput->Write(); }
    if (fDecayVertexOutput) { fDecayVertexOutput->Write(); }
    fEarthHitOutput->Write();
    // close file
    fFile->Close(option);
    delete fFile;
    // reset output
    fPrimaryVertexOutput.reset();
    fDecayVertexOutput.reset();
    fEarthHitOutput.reset();
}

} // namespace AirShower
