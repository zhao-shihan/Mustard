#pragma once

namespace CLHEP {
class HepRandomEngine;
} // namespace CLHEP
class TRandom;

namespace Mustard::inline Utility {

auto MPIReseedRandomEngine(CLHEP::HepRandomEngine* clhepRng = {}, TRandom* tRandom = {}) -> void;

} // namespace Mustard::inline Utility
