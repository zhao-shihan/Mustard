namespace Mustard::inline Extension::Geant4X::inline Process {

template<TargetForMuoniumPhysics ATarget>
MuoniumFormation<ATarget>::MuoniumFormation() :
    NonMoveableBase{},
    G4VRestProcess{"MuoniumFormation", fUserDefined},
    fConversionProbability{0},
    fParticleChange{},
    fMessengerRegister{this} {
    pParticleChange = &fParticleChange;
}

template<TargetForMuoniumPhysics ATarget>
auto MuoniumFormation<ATarget>::IsApplicable(const G4ParticleDefinition& particle) -> G4bool {
    return &particle == G4MuonPlus::Definition();
}

template<TargetForMuoniumPhysics ATarget>
auto MuoniumFormation<ATarget>::AtRestDoIt(const G4Track& track, const G4Step&) -> G4VParticleChange* {
    using namespace PhysicalConstant;
    auto& rng{*G4Random::getTheEngine()};

    fParticleChange.Initialize(track);
    // The dynamic particle
    const auto muoniumDynamicParticle{new G4DynamicParticle{*track.GetDynamicParticle()}};
    // Determine whether the transition can be observed
    const auto conversion{rng.flat() < fConversionProbability};
    // Use antimuonium if it will be a conversion... not exactly true since it is not pure antimuonium but mix state. OK for now though.
    // FIXME: Try always using muonium but behave like mix state in each interaction.
    muoniumDynamicParticle->SetDefinition(conversion ? static_cast<G4ParticleDefinition*>(Geant4X::Antimuonium::Definition()) :
                                                       static_cast<G4ParticleDefinition*>(Geant4X::Muonium::Definition()));
    // Must pre-assign the decay time to ensure correct behaviour of transport and decay
    // (transport process use this to determine when to stop flight,
    //  instead of relying on G4 tracking mechanism. See MuoniumTransport process for detail.)
    if (conversion) {
        // use the muonium conversion time spectrum (prop. to t^2*exp(-t/tau))
        const auto [tStar, converged]{muc::find_root::secant(
            // CDF - x
            [x = rng.flat()](const auto t) {
                const auto cdf{1 - std::exp(-t) * Math::Polynomial({2, 2, 1}, t) / 2};
                return cdf - x;
            },
            // most probable t*
            2.)};
        if (not converged) {
            Env::PrintLnError("MuoniumFormation::AtRestDoIt: antimuonium decay time disconverged");
        }
        muoniumDynamicParticle->SetPreAssignedDecayProperTime(tStar * muonium_lifetime);
    } else {
        // use standard exp decay
        muoniumDynamicParticle->SetPreAssignedDecayProperTime(G4RandExponential::shoot(&rng, muonium_lifetime));
    }
    // Sampling momentum according to boltzmann distribution
    const auto temperature{track.GetVolume()->GetLogicalVolume()->GetMaterial()->GetTemperature()}; // clang-format off
    const auto momentum{std::sqrt(muonium_mass_c2 * k_Boltzmann * temperature) *
                        G4ThreeVector{G4RandGauss::shoot(&rng),
                                      G4RandGauss::shoot(&rng),
                                      G4RandGauss::shoot(&rng)}}; // clang-format on
    // Set momentum and energy
    muoniumDynamicParticle->SetMomentum(momentum);
    muoniumDynamicParticle->SetKineticEnergy(momentum.mag2() / (2 * muonium_mass_c2));
    // Kill the muon, form the (anti-)muonium
    fParticleChange.ProposeTrackStatus(fStopAndKill);
    fParticleChange.AddSecondary(new G4Track{muoniumDynamicParticle, track.GetGlobalTime(), track.GetPosition()});
    // Clean
    ClearNumberOfInteractionLengthLeft();
    return &fParticleChange;
}

template<TargetForMuoniumPhysics ATarget>
auto MuoniumFormation<ATarget>::GetMeanLifeTime(const G4Track& track, G4ForceCondition*) -> G4double {
    const auto mpt{track.GetNextMaterial()->GetMaterialPropertiesTable()};
    if (mpt == nullptr or not mpt->ConstPropertyExists("MUONIUM_FORM_PROB")) {
        return std::numeric_limits<double>::max();
    }
    if (G4Random::getTheEngine()->flat() < mpt->GetConstProperty("MUONIUM_FORM_PROB") and
        ATarget::Instance().Contain(track.GetPosition())) {
        return std::numeric_limits<double>::min();
    }
    return std::numeric_limits<double>::max();
}

} // namespace Mustard::inline Extension::Geant4X::inline Process
