#include "AirShower/Action/RunAction.h++"
#include "AirShower/Analysis.h++"

#include "G4Run.hh"

namespace AirShower::inline Action {

auto RunAction::BeginOfRunAction(const G4Run* run) -> void {
    Analysis::Instance().RunBegin(run->GetRunID());
}

auto RunAction::EndOfRunAction(const G4Run*) -> void {
    Analysis::Instance().RunEnd();
}

} // namespace AirShower::inline Action
