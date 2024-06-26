#include "AirShower/Action/ActionInitialization.h++"
#include "AirShower/Action/EventAction.h++"
#include "AirShower/Action/PrimaryGeneratorAction.h++"
#include "AirShower/Action/RunAction.h++"
#include "AirShower/Action/TrackingAction.h++"

namespace AirShower::inline Action {

auto ActionInitialization::Build() const -> void {
    SetUserAction(new RunAction);
    SetUserAction(new PrimaryGeneratorAction);
    SetUserAction(new EventAction);
    SetUserAction(new TrackingAction);
}

} // namespace AirShower
