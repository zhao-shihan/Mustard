// -*- C++ -*-
//
// Copyright (C) 2020-2025  The Mustard development team
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

#include "Mustard/CLHEPX/Random/Xoshiro.h++"
#include "Mustard/CLI/Module/MonteCarloModule.h++"
#include "Mustard/Parallel/ReseedRandomEngine.h++"
#include "Mustard/ROOTX/Math/Xoshiro.h++"
#include "Mustard/Utility/UseXoshiro.h++"

#include "CLHEP/Random/Random.h"

#include "TRandom.h"

#include "mplr/mplr.hpp"

#include "muc/array"

#include <memory>
#include <random>

namespace Mustard::inline Utility {

#define MUSTARD_UTILITY_USE_XOSHIRO_DEFINITION(NBit)                                             \
                                                                                                 \
    template<>                                                                                   \
    struct UseXoshiro<NBit>::Random {                                                            \
        CLHEPX::Random::Xoshiro##NBit##StarStar clhep;                                           \
        ROOTX::Math::Xoshiro##NBit##PlusPlus root;                                               \
    };                                                                                           \
                                                                                                 \
    template<>                                                                                   \
    UseXoshiro<NBit>::UseXoshiro(muc::optional_ref<const CLI::CLI<>> cli) :                      \
        fRandom{std::make_unique<Random>()} {                                                    \
        /* Set random engines */                                                                 \
        CLHEP::HepRandom::setTheEngine(&fRandom->clhep);                                         \
        delete gRandom, gRandom = &fRandom->root;                                                \
        try {                                                                                    \
            /* Seed random engines from CLI (if option set) */                                   \
            dynamic_cast<const CLI::MonteCarloModule&>(cli.value().get()).SeedRandomIfFlagged(); \
        } catch (const std::exception&) {                                                        \
            /* Default seeding. Try to decorrelate Xoshiro++ from Xoshiro** */                   \
            gRandom->SetSeed(std::mt19937_64{std::bit_cast<std::uint64_t>(                       \
                muc::array2u32{gRandom->Integer(-1) + 1, gRandom->Integer(-1) + 1})}());         \
        }                                                                                        \
        /* Reseed in parallel computing */                                                       \
        Parallel::ReseedRandomEngine();                                                          \
    }                                                                                            \
                                                                                                 \
    template<>                                                                                   \
    UseXoshiro<NBit>::~UseXoshiro() {                                                            \
        CLHEP::HepRandom::setTheEngine(nullptr);                                                 \
        gRandom = nullptr;                                                                       \
    }

MUSTARD_UTILITY_USE_XOSHIRO_DEFINITION(256)
MUSTARD_UTILITY_USE_XOSHIRO_DEFINITION(512)

#undef MUSTARD_UTILITY_USE_XOSHIRO_DEFINITION

} // namespace Mustard::inline Utility
