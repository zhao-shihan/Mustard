#include "AirShower/Action/EventAction.h++"
#include "AirShower/Action/PrimaryGeneratorAction.h++"
#include "AirShower/Action/TrackingAction.h++"
#include "AirShower/Analysis.h++"

#include "G4Event.hh"

namespace AirShower::inline Action {

auto EventAction::BeginOfEventAction(const G4Event*) -> void {
    if (auto& trackingAction{TrackingAction::Instance()};
        trackingAction.SaveDecayVertexData()) {
        trackingAction.ClearDecayVertexData();
    }
}

auto EventAction::EndOfEventAction(const G4Event*) -> void {
    auto& analysis{Analysis::Instance()};
    if (const auto& trackingAction{TrackingAction::Instance()};
        trackingAction.SaveDecayVertexData()) {
        analysis.SubmitDecayVertexData(trackingAction.DecayVertexData());
    }
    analysis.EventEnd();
}

} // namespace AirShower::inline Action
