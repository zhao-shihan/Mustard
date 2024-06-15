#pragma once

#include "Mustard/Env/BasicEnv.h++"
#include "Mustard/Env/Print.h++"
#include "Mustard/Math/Polynomial.h++"
#include "Mustard/Utility/LiteralUnit.h++"
#include "Mustard/Utility/PhysicalConstant.h++"

#include "G4DecayProducts.hh"
#include "G4ParticleDefinition.hh"
#include "G4RandomDirection.hh"
#include "G4String.hh"
#include "G4VDecayChannel.hh"
#include "Randomize.hh"

#include "muc/ceta_string"
#include "muc/math"
#include "muc/numeric"

#include "gsl/gsl"

#include "fmt/format.h"

#include <cmath>
#include <concepts>
#include <cstdio>
#include <stdexcept>
#include <vector>

namespace Mustard::inline Extension::Geant4X::inline DecayChannel {

template<std::derived_from<G4VDecayChannel> AMuonDecayChannel, muc::ceta_string AName>
class AsMuoniumDecayChannel : public AMuonDecayChannel {
public:
    AsMuoniumDecayChannel(const G4String& parentName, G4double br, G4int verbose = 1);

    auto DecayIt(G4double) -> G4DecayProducts* override;

private:
    int fAtomicShellProductIndex;
};

} // namespace Mustard::inline Extension::Geant4X::inline DecayChannel

#include "Mustard/Extension/Geant4X/DecayChannel/AsMuoniumDecayChannel.inl"
