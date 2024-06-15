#include "Mustard/Extension/Geant4X/Physics/DecayPhysicsBase.h++"

#include "G4DecayTable.hh"

#include <algorithm>
#include <limits>
#include <stdexcept>

namespace Mustard::inline Extension::Geant4X::inline Physics {

auto DecayPhysicsBase::UpdateDecayBRFor(const G4ParticleDefinition* particle) -> void {
    const auto decay{particle->GetDecayTable()};
    // set rare decay mode first
    AssignRareDecayBR(decay);
    // then set main decay mode
    double mainDecayBR{1};
    for (auto i{1}; i < decay->entries(); ++i) {
        mainDecayBR -= decay->GetDecayChannel(i)->GetBR();
    }
    if (mainDecayBR < -std::numeric_limits<double>::epsilon()) {
        decay->DumpInfo();
        throw std::runtime_error{"Impossible to normalize decay branching ratio (sum of rare channel BR > 1)"};
    }
    decay->GetDecayChannel(0)->SetBR(std::max(0., mainDecayBR));
}

} // namespace Mustard::inline Extension::Geant4X::inline Physics
