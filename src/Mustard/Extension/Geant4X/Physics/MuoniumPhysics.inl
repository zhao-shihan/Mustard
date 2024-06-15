namespace Mustard::inline Extension::Geant4X::inline Physics {

template<TargetForMuoniumPhysics ATarget>
MuoniumPhysics<ATarget>::MuoniumPhysics(G4int verbose) :
    NonMoveableBase{},
    G4VPhysicsConstructor{"MuoniumPhysics"} {
    verboseLevel = verbose;
}

template<TargetForMuoniumPhysics ATarget>
auto MuoniumPhysics<ATarget>::ConstructParticle() -> void {
    G4EmBuilder::ConstructMinimalEmSet();
    G4MuonPlus::Definition();
    Muonium::Definition();
    Antimuonium::Definition();
}

template<TargetForMuoniumPhysics ATarget>
auto MuoniumPhysics<ATarget>::ConstructProcess() -> void {
    const auto muoniumFormation{new MuoniumFormation<ATarget>};
    const auto muoniumTransport{new MuoniumTransport<ATarget>};

    const auto muonPlus{G4MuonPlus::Definition()->GetProcessManager()};
    muonPlus->AddRestProcess(muoniumFormation);

    const auto muonium{Muonium::Definition()->GetProcessManager()};
    muonium->AddContinuousProcess(muoniumTransport);

    const auto antiMuonium{Antimuonium::Definition()->GetProcessManager()};
    antiMuonium->AddContinuousProcess(muoniumTransport);
}

} // namespace Mustard::inline Extension::Geant4X::inline Physics
