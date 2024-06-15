#pragma once

#include "Mustard/Extension/Geant4X/Run/MPIRunMessenger.h++"
#include "Mustard/Extension/MPIX/Execution/Executor.h++"
#include "Mustard/Math/Statistic.h++"
#include "Mustard/Utility/NonMoveableBase.h++"

#include "G4RunManager.hh"

#include "mpi.h"

#include "gsl/gsl"

#include <chrono>
#include <memory>

namespace Mustard::inline Extension::Geant4X::inline Run {

namespace internal {

class FlipG4cout {
protected:
    FlipG4cout();
};

class PreG4RunManagerInitFlipG4cout : private FlipG4cout {};

class PostG4RunManagerInitFlipG4cout : private FlipG4cout {};

} // namespace internal

class MPIRunManager : public NonMoveableBase,
                      private internal::PreG4RunManagerInitFlipG4cout,
                      public G4RunManager,
                      private internal::PostG4RunManagerInitFlipG4cout {
public:
    MPIRunManager();
    virtual ~MPIRunManager() = default;

    static auto GetRunManager() -> auto { return static_cast<MPIRunManager*>(G4RunManager::GetRunManager()); }

    auto PrintProgress(G4bool b) -> void { fExecutor.PrintProgress(b), printModulo = -1; }
    auto PrintProgressModulo(G4int mod) -> void { fExecutor.PrintProgressModulo(mod), printModulo = -1; }

    virtual auto BeamOn(G4int nEvent, gsl::czstring macroFile = nullptr, G4int nSelect = -1) -> void override;
    virtual auto ConfirmBeamOnCondition() -> G4bool override;
    virtual auto DoEventLoop(G4int nEvent, const char* macroFile, G4int nSelect) -> void override;

    auto PrintRunSummary() const -> void { fExecutor.PrintExecutionSummary(); }

private:
    MPIX::Executor<G4int> fExecutor;

    MPIRunMessenger::Register<MPIRunManager> fMessengerRegister;
};

} // namespace Mustard::inline Extension::Geant4X::inline Run
