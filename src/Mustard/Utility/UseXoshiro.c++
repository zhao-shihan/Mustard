#include "Mustard/Extension/CLHEPX/Random/Xoshiro.h++"
#include "Mustard/Extension/ROOTX/Math/Xoshiro.h++"
#include "Mustard/Utility/UseXoshiro.h++"

#include "CLHEP/Random/Random.h"

#include "TRandom.h"

#include "muc/array"

#include <memory>
#include <random>

namespace Mustard::inline Utility {

#define MUSTARD_UTILITY_USE_XOSHIRO_DEFINITION(NBit)                                     \
                                                                                      \
    template<>                                                                        \
    struct UseXoshiro<NBit>::Random {                                                 \
        CLHEPX::Random::Xoshiro##NBit##SS clhep;                                      \
        ROOTX::Math::Xoshiro##NBit##PP root;                                          \
    };                                                                                \
                                                                                      \
    template<>                                                                        \
    UseXoshiro<NBit>::UseXoshiro() :                                                  \
        fRandom{std::make_unique_for_overwrite<Random>()} {                           \
        /* Set random engines */                                                      \
        CLHEP::HepRandom::setTheEngine(&fRandom->clhep);                              \
        delete gRandom, gRandom = &fRandom->root;                                     \
        /* Try to decorrelate Xoshiro++ from Xoshiro** */                             \
        gRandom->SetSeed(std::mt19937_64{std::bit_cast<std::uint64_t>(                \
            muc::array2u32{gRandom->Integer(-1) + 1, gRandom->Integer(-1) + 1})}()); \
    }                                                                                 \
                                                                                      \
    template<>                                                                        \
    UseXoshiro<NBit>::~UseXoshiro() {                                                 \
        CLHEP::HepRandom::setTheEngine(nullptr);                                      \
        gRandom = nullptr;                                                            \
    }

MUSTARD_UTILITY_USE_XOSHIRO_DEFINITION(256)
MUSTARD_UTILITY_USE_XOSHIRO_DEFINITION(512)

#undef MUSTARD_UTILITY_USE_XOSHIRO_DEFINITION

} // namespace Mustard::inline Utility
