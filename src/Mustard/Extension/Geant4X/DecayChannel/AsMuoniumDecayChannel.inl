// -*- C++ -*-
//
// Copyright 2020-2024  The Mustard development team
//
// This file is part of Mustard, an offline software framework for HEP experiments.
//
// Mustard is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// Mustard is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// Mustard. If not, see <https://www.gnu.org/licenses/>.

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
        Throw<std::invalid_argument>(fmt::format("Parent particle is not muonium or anti_muonium but {}", parentName));
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

    PrintLn<'V'>("AsMuoniumDecayChannel::DecayIt");

    G4ThreeVector p{};
    for (auto i{0};; ++i) {
        if (i == 100) {
            PrintError("Atomic shell e+/e- momentum disconverged");
            break;
        }
        const auto [pStar, converged]{muc::find_root::secant(
            // CDF - x
            [x = G4UniformRand()](auto p) {
                const auto cdf{(2 / 3_pi) * (muc::polynomial({0, -3, 0, 8, 0, 3}, p) / muc::pow<3>(p * p + 1) +
                                             3 * std::atan(p))};
                return cdf - x;
            },
            // most probable p*
            27 / 8_pi)};
        if (converged) {
            p = fine_structure_const * muonium_reduced_mass_c2 * pStar * G4RandomDirection();
            break;
        }
    }

    const auto products{AMuonDecayChannel::DecayIt(muon_mass_c2)};
    products->Boost(-p.x() / muon_mass_c2, -p.y() / muon_mass_c2, -p.z() / muon_mass_c2); // recoil boost
    products->PushProducts(new G4DynamicParticle{this->G4MT_daughters[fAtomicShellProductIndex], p});

    PrintLn<'V'>("AsMuoniumDecayChannel::DecayIt\n"
                 "\tCreate decay products in rest frame.");
    if (Env::VerboseLevelReach<'V'>()) { products->DumpInfo(); }

    return products;
}

} // namespace Mustard::inline Extension::Geant4X::inline DecayChannel
