#pragma once

#include "AirShower/Data/DecayVertex.h++"
#include "AirShower/Data/PrimaryVertex.h++"
#include "AirShower/Hit/EarthHit.h++"
#include "AirShower/Messenger/AnalysisMessenger.h++"

#include "Mustard/Data/Output.h++"
#include "Mustard/Data/Tuple.h++"
#include "Mustard/Env/Memory/PassiveSingleton.h++"

#include "G4Types.hh"

#include "muc/ptr_vector"

#include "gsl/gsl"

#include <filesystem>
#include <memory>
#include <utility>

class TFile;

namespace AirShower {

class Analysis final : public Mustard::Env::Memory::PassiveSingleton<Analysis> {
public:
    Analysis();

    auto FilePath(std::filesystem::path path) -> void { fFilePath = std::move(path); }
    auto FileMode(std::string mode) -> void { fFileMode = std::move(mode); }

    auto RunBegin(int runID) -> void;

    auto SubmitPrimaryVertexData(const muc::unique_ptr_vector<Mustard::Data::Tuple<Data::PrimaryVertex>>& data) -> void { fPrimaryVertex = &data; }
    auto SubmitDecayVertexData(const muc::unique_ptr_vector<Mustard::Data::Tuple<Data::DecayVertex>>& data) -> void { fDecayVertex = &data; }
    auto SubmitVirtualHC(const std::vector<EarthHit*>& hc) -> void { fEarthHit = &hc; }
    auto EventEnd() -> void;

    auto RunEnd(Option_t* option = {}) -> void;

private:
    std::filesystem::path fFilePath;
    std::string fFileMode;

    std::filesystem::path fLastUsedFullFilePath;

    gsl::owner<TFile*> fFile;
    std::optional<Mustard::Data::Output<Data::PrimaryVertex>> fPrimaryVertexOutput;
    std::optional<Mustard::Data::Output<Data::DecayVertex>> fDecayVertexOutput;
    std::optional<Mustard::Data::Output<Data::EarthHit>> fEarthHitOutput;

    const muc::unique_ptr_vector<Mustard::Data::Tuple<Data::PrimaryVertex>>* fPrimaryVertex;
    const muc::unique_ptr_vector<Mustard::Data::Tuple<Data::DecayVertex>>* fDecayVertex;
    const std::vector<EarthHit*>* fEarthHit;

    AnalysisMessenger::Register<Analysis> fMessengerRegister;
};

} // namespace AirShower
