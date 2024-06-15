namespace Mustard::inline Extension::Geant4X::inline DecayChannel {

template<std::derived_from<G4VDecayChannel> AMuonDecayChannel, muc::ceta_string AName>
AsMuoniumDecayChannel<AMuonDecayChannel, AName>::AsMuoniumDecayChannel(const G4String& parentName, G4double br, G4int verbose) :
    AMuonDecayChannel{parentName == "muonium" ? "mu+" : "mu-", br},
    fAtomicShellProductIndex{this->numberOfDaughters} {
    this->SetVerboseLevel(verbose);
    // save muon decay info
    std::vector<G4String> daughter;
    daughter.reserve(this->numberOfDaughters + 1);
    for (int i{}; i < this->numberOfDaughters; ++i) {
        daughter.emplace_back(this->GetDaughterName(i));
    }
    if (parentName == "muonium") {
        daughter.emplace_back("e-");
    } else if (parentName == "anti_muonium") {
        daughter.emplace_back("e+");
    } else {
        throw std::invalid_argument{fmt::format("AsMuoniumDecayChannel: parent particle is not muonium or anti_muonium but {}", parentName)};
    }
    // set muonium decay info
    this->kinematics_name = AName.sv();
    this->SetParent(parentName);
    this->SetBR(br);
    this->SetNumberOfDaughters(daughter.size());
    for (gsl::index i{}; i < ssize(daughter); ++i) {
        this->SetDaughter(i, daughter[i]);
    }
}

template<std::derived_from<G4VDecayChannel> AMuonDecayChannel, muc::ceta_string AName>
auto AsMuoniumDecayChannel<AMuonDecayChannel, AName>::DecayIt(G4double) -> G4DecayProducts* {
    using namespace LiteralUnit::MathConstantSuffix;
    using namespace PhysicalConstant;

    Env::PrintLn<'V'>("AsMuoniumDecayChannel::DecayIt");

    const auto [pStar, converged]{muc::find_root::secant(
        // CDF - x
        [x = G4UniformRand()](auto p) {
            const auto cdf{(2 / 3_pi) * (Math::Polynomial({0, -3, 0, 8, 0, 3}, p) / muc::pow<3>(p * p + 1) +
                                         3 * std::atan(p))};
            return cdf - x;
        },
        // most probable p*
        27 / 8_pi)};
    if (not converged) {
        Env::PrintLnError("AsMuoniumDecayChannel: atomic shell e+/e- momentum disconverged");
    }
    const auto p{fine_structure_const * muonium_reduced_mass_c2 * pStar * G4RandomDirection()};

    const auto products{AMuonDecayChannel::DecayIt(muon_mass_c2)};
    products->Boost(-p.x() / muon_mass_c2, -p.y() / muon_mass_c2, -p.z() / muon_mass_c2); // recoil boost
    products->PushProducts(new G4DynamicParticle{this->G4MT_daughters[fAtomicShellProductIndex], p});

    Env::PrintLn<'V'>("AsMuoniumDecayChannel::DecayIt\n"
                      "\tCreate decay products in rest frame.");
    if (Env::VerboseLevelReach<'V'>()) { products->DumpInfo(); }

    return products;
}

} // namespace Mustard::inline Extension::Geant4X::inline DecayChannel
