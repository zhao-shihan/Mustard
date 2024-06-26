#include "AirShower/Action/PrimaryGeneratorAction.h++"
#include "AirShower/Action/TrackingAction.h++"
#include "AirShower/Analysis.h++"

#include "G4Event.hh"
#include "G4EventManager.hh"
#include "G4ProcessType.hh"
#include "G4SteppingManager.hh"
#include "G4Track.hh"
#include "G4TrackingManager.hh"
#include "G4VProcess.hh"

#include <cassert>
#include <vector>

namespace AirShower::inline Action {

TrackingAction::TrackingAction() :
    PassiveSingleton{},
    G4UserTrackingAction{},
    fSaveDecayVertexData{true},
    fDecayVertexData{},
    fMessengerRegister{this} {}

auto TrackingAction::PostUserTrackingAction(const G4Track* track) -> void {
    if (fSaveDecayVertexData) { UpdateDecayVertexData(*track); }
}

auto TrackingAction::UpdateDecayVertexData(const G4Track& track) -> void {
    if (auto& eventManager{*G4EventManager::GetEventManager()};
        eventManager.GetTrackingManager()
            ->GetSteppingManager()
            ->GetfCurrentProcess()
            ->GetProcessType() == fDecay) {
        std::vector<int> secondaryPDGID;
        secondaryPDGID.reserve(track.GetStep()->GetSecondary()->size());
        for (auto&& sec : *track.GetStep()->GetSecondary()) {
            secondaryPDGID.emplace_back(sec->GetParticleDefinition()->GetPDGEncoding());
        }
        auto& vertex{fDecayVertexData.emplace_back()};
        Get<"EvtID">(vertex) = eventManager.GetConstCurrentEvent()->GetEventID();
        Get<"TrkID">(vertex) = track.GetTrackID();
        Get<"PDGID">(vertex) = track.GetParticleDefinition()->GetPDGEncoding();
        Get<"SecPDGID">(vertex) = std::move(secondaryPDGID);
        Get<"t">(vertex) = track.GetGlobalTime();
        Get<"x">(vertex) = track.GetPosition();
        Get<"Ek">(vertex) = track.GetKineticEnergy();
        Get<"p">(vertex) = track.GetMomentum();
    }
}

} // namespace AirShower::inline Action
