#include "AirShower/Analysis.h++"
#include "AirShower/Hit/EarthHit.h++"
#include "AirShower/SD/EarthSD.h++"

#include "G4Event.hh"
#include "G4EventManager.hh"
#include "G4HCofThisEvent.hh"
#include "G4RotationMatrix.hh"
#include "G4SDManager.hh"
#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4String.hh"
#include "G4TrackStatus.hh"

#include "muc/math"

#include <cmath>
#include <string_view>

namespace AirShower::inline SD {

EarthSD::EarthSD(const G4String& sdName) :
    Mustard::NonMoveableBase{},
    G4VSensitiveDetector{sdName},
    fHitsCollection{} {
    collectionName.insert(sdName + "HC");
}

auto EarthSD::Initialize(G4HCofThisEvent* hitsCollectionOfThisEvent) -> void {
    fHitsCollection = new EarthHitCollection{SensitiveDetectorName, collectionName[0]};
    auto hitsCollectionID{G4SDManager::GetSDMpointer()->GetCollectionID(fHitsCollection)};
    hitsCollectionOfThisEvent->AddHitsCollection(hitsCollectionID, fHitsCollection);
}

auto EarthSD::ProcessHits(G4Step* theStep, G4TouchableHistory*) -> G4bool {
    const auto& step{*theStep};
    const auto& track{*step.GetTrack()};
    const auto& particle{*track.GetDefinition()};
    const auto pdgID{particle.GetPDGEncoding()};

    enum { vE = 12,
           vMu = 14,
           vTau = 16 };
    if (const auto absPDGID{muc::abs(pdgID)};
        absPDGID == vE or absPDGID == vMu or absPDGID == vTau) {
        return false;
    }

    const auto& preStepPoint{*step.GetPreStepPoint()};
    // calculate (E0, p0)
    const auto vertexEk{track.GetVertexKineticEnergy()};
    const auto vertexMomentum{track.GetVertexMomentumDirection() * std::sqrt(vertexEk * (vertexEk + 2 * particle.GetPDGMass()))};
    // calculate theta
    const auto p{preStepPoint.GetMomentum()};
    const auto theta{std::atan2(p.perp(), -p.z())};
    const auto phi{std::atan2(p.y(), p.x())};
    // track creator process
    const auto creatorProcess{track.GetCreatorProcess()};
    // new a hit
    auto hit{new EarthHit};
    Get<"EvtID">(*hit) = G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetEventID();
    Get<"TrkID">(*hit) = track.GetTrackID();
    Get<"PDGID">(*hit) = pdgID;
    Get<"t">(*hit) = preStepPoint.GetGlobalTime();
    Get<"t0">(*hit) = track.GetGlobalTime() - track.GetLocalTime();
    Get<"x">(*hit) = preStepPoint.GetPosition();
    Get<"x0">(*hit) = track.GetVertexPosition();
    Get<"Ek">(*hit) = preStepPoint.GetKineticEnergy();
    Get<"Ek0">(*hit) = vertexEk;
    Get<"p">(*hit) = p;
    Get<"p0">(*hit) = vertexMomentum;
    Get<"theta">(*hit) = theta;
    Get<"phi">(*hit) = phi;
    Get<"len">(*hit) = track.GetTrackLength();
    Get<"ParentTrkID">(*hit) = track.GetParentID();
    *Get<"CreatProc">(*hit) = creatorProcess ? std::string_view{creatorProcess->GetProcessName()} : "|0>";
    fHitsCollection->insert(hit);

    // kill the track
    step.GetTrack()->SetTrackStatus(fStopAndKill);

    return true;
}

auto EarthSD::EndOfEvent(G4HCofThisEvent*) -> void {
    Analysis::Instance().SubmitVirtualHC(*fHitsCollection->GetVector());
}

} // namespace AirShower::inline SD
