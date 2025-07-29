#pragma once

#include "Mustard/Env/MPIEnv.h++"
#include "Mustard/Env/Memory/PassiveSingleton.h++"
#include "Mustard/Geant4X/Utility/ConvertGeometry.h++"
#include "Mustard/Parallel/ProcessSpecificPath.h++"
#include "Mustard/Simulation/AnalysisBaseMessenger.h++"
#include "Mustard/Utility/PrettyLog.h++"

#include "TFile.h"
#include "TMacro.h"

#include "muc/ceta_string"

#include "gsl/gsl"

#include "fmt/core.h"

#include <concepts>
#include <filesystem>
#include <stdexcept>
#include <utility>

namespace Mustard::Simulation {

template<typename ADerived, muc::ceta_string AAppName>
class AnalysisBase : public Env::Memory::PassiveSingleton<ADerived> {
public:
    [[deprecated]] AnalysisBase();
    AnalysisBase(ADerived* self);
    virtual ~AnalysisBase() = default;

    auto FilePath(std::filesystem::path path) -> void { fFilePath = std::move(path); }
    auto FileMode(std::string mode) -> void { fFileMode = std::move(mode); }

    auto RunBeginAction(int runID) -> void;
    auto EventEndAction() -> void;
    auto RunEndAction(int runID) -> void;

private:
    virtual auto RunBeginUserAction(int runID) -> void = 0;
    virtual auto EventEndUserAction() -> void = 0;
    virtual auto RunEndUserAction(int runID) -> void = 0;

private:
    std::filesystem::path fFilePath;
    std::string fFileMode;

    std::filesystem::path fLastUsedFullFilePath;
    gsl::owner<TFile*> fFile;

    AnalysisBaseMessenger<ADerived>::template Register<ADerived> fMessengerRegister;
};

} // namespace Mustard::Simulation

#include "Mustard/Simulation/AnalysisBase.inl"
