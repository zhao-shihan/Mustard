#pragma once

#include "AirShower/Data/DecayVertex.h++"
#include "AirShower/Messenger/AnalysisMessenger.h++"

#include "Mustard/Data/Tuple.h++"
#include "Mustard/Env/Memory/PassiveSingleton.h++"

#include "G4UserTrackingAction.hh"

#include "muc/ptr_vector"

#include <memory>
#include <vector>

namespace AirShower::inline Action {

class TrackingAction final : public Mustard::Env::Memory::PassiveSingleton<TrackingAction>,
                             public G4UserTrackingAction {
public:
    TrackingAction();

    auto PostUserTrackingAction(const G4Track* track) -> void override;

    auto SaveDecayVertexData() const -> auto { return fSaveDecayVertexData; }
    auto SaveDecayVertexData(bool val) -> void { fSaveDecayVertexData = val; }

    auto ClearDecayVertexData() -> void { return fDecayVertexData.clear(); }
    auto DecayVertexData() const -> const auto& { return fDecayVertexData; }

private:
    auto UpdateDecayVertexData(const G4Track& track) -> void;

private:
    bool fSaveDecayVertexData;

    muc::unique_ptr_vector<Mustard::Data::Tuple<Data::DecayVertex>> fDecayVertexData;

    AnalysisMessenger::Register<TrackingAction> fMessengerRegister;
};

} // namespace AirShower::inline Action
