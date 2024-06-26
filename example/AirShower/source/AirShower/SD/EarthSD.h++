#pragma once

#include "AirShower/Hit/EarthHit.h++"

#include "Mustard/Utility/NonMoveableBase.h++"

#include "G4VSensitiveDetector.hh"

namespace AirShower::inline SD {

class EarthSD : public Mustard::NonMoveableBase,
                  public G4VSensitiveDetector {
public:
    EarthSD(const G4String& sdName);

    virtual auto Initialize(G4HCofThisEvent* hitsCollection) -> void override;
    virtual auto ProcessHits(G4Step* theStep, G4TouchableHistory*) -> G4bool override;
    virtual auto EndOfEvent(G4HCofThisEvent*) -> void override;

protected:
    EarthHitCollection* fHitsCollection;
};

} // namespace AirShower::inline SD
