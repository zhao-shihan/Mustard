// -*- C++ -*-
//
// Copyright (C) 2020-2026  Shihan Zhao and contributors
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

namespace Mustard::ROOTX::Math {

template<Mustard::Random::RandomNumberEngineOrReference R>
AsTRandom<R>::AsTRandom()
    requires(not std::is_reference_v<R>)
    :
    TRandom{},
    Base{},
    NoncopyableBase{},
    fGaus{} {
    fSeed = 0; // The engine is at its default state, so its seed is unknown.
}

template<Mustard::Random::RandomNumberEngineOrReference R>
AsTRandom<R>::AsTRandom(SeedType seed)
    requires(not std::is_reference_v<R>)
    :
    TRandom{},
    Base{},
    NoncopyableBase{},
    fGaus{} {
    SetSeed(static_cast<unsigned long>(seed)); // as TRandom(seed) does
}

template<Mustard::Random::RandomNumberEngineOrReference R>
AsTRandom<R>::AsTRandom(EngineType& engine)
    requires(std::is_reference_v<R>)
    :
    TRandom{},
    Base{engine},
    NoncopyableBase{},
    fGaus{} {
    fSeed = 0; // The engine belongs to someone else, so its seed is unknown.
}

template<Mustard::Random::RandomNumberEngineOrReference R>
auto AsTRandom<R>::SetSeed(unsigned long seed) -> void {
    while (seed == 0) {
        // ROOT's SetSeed(0) asks for a non-deterministic seed
        seed = Random::RandomDevice{}();
    }
    Engine().seed(static_cast<SeedType>(seed));
    fGaus.Reset(); // a cached Gaussian variate belongs to the previous stream
    fSeed = static_cast<unsigned>(seed);
}

template<Mustard::Random::RandomNumberEngineOrReference R>
auto AsTRandom<R>::RndmArray(int n, float* array) -> void {
    std::ranges::generate_n(array, n, [this] { return DrawOpen01<float>(); });
}

template<Mustard::Random::RandomNumberEngineOrReference R>
auto AsTRandom<R>::RndmArray(int n, double* array) -> void {
    std::ranges::generate_n(array, n, [this] { return DrawOpen01<double>(); });
}

template<Mustard::Random::RandomNumberEngineOrReference R>
template<std::floating_point T>
auto AsTRandom<R>::DrawOpen01() -> T {
    T value;
    do {
        value = Random::Uniform<T>{}(Engine());
    } while (value == 0); // ROOT excludes 0 from ]0, 1]
    return value;
}

} // namespace Mustard::ROOTX::Math
