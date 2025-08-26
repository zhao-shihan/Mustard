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

namespace Mustard::CLHEPX::Random {

template<Math::Random::UniformPseudoRandomBitGenerator PRBG>
Wrap<PRBG>::Wrap(long seed) :
    HepRandomEngine{},
    fPRBG{static_cast<typename PRBG::SeedType>(seed)} {
    theSeed = seed;
    theSeeds = &theSeed;
}

template<Math::Random::UniformPseudoRandomBitGenerator PRBG>
auto Wrap<PRBG>::flatArray(const int size, double* vect) -> void {
    for (int i = 0; i < size; ++i) {
        vect[i] = flat();
    }
}

template<Math::Random::UniformPseudoRandomBitGenerator PRBG>
auto Wrap<PRBG>::setSeed(long seed, int) -> void {
    fPRBG.Seed(seed);
    theSeed = seed;
    theSeeds = &theSeed;
}

template<Math::Random::UniformPseudoRandomBitGenerator PRBG>
auto Wrap<PRBG>::setSeeds(const long* seeds, int) -> void {
    fPRBG.Seed(*seeds);
    theSeed = *seeds;
    theSeeds = seeds;
}

template<Math::Random::UniformPseudoRandomBitGenerator PRBG>
auto Wrap<PRBG>::saveStatus(gsl::czstring filename) const -> void {
    std::ofstream os(filename, std::ios::out);
    if (os.is_open()) {
        put(os);
    } else {
        PrintError(fmt::format("Cannot open '{}', nothing was done. (Wrap<PRBG>::name(): {})",
                               filename, name()));
    }
}

template<Math::Random::UniformPseudoRandomBitGenerator PRBG>
auto Wrap<PRBG>::restoreStatus(gsl::czstring filename) -> void {
    std::ifstream is(filename, std::ios::in);
    if (is.is_open()) {
        get(is);
    } else {
        PrintError(fmt::format("Cannot open '{}', nothing was done. (Wrap<PRBG>::name(): {})",
                               filename, name()));
    }
}

template<Math::Random::UniformPseudoRandomBitGenerator PRBG>
auto Wrap<PRBG>::showStatus() const -> void {
    constexpr std::string_view split = "----------------------------------------------------------------";
    const auto engineName{name()};
    const auto firstSplit{static_cast<gsl::index>(split.length()) -          // length(split) -
                          static_cast<gsl::index>(engineName.length() + 9)}; // length(" " + engineName + " status ")
    const auto firstSplitL{std::max<gsl::index>(3, firstSplit / 2)};
    const auto firstSplitR{std::max<gsl::index>(3, muc::even(firstSplit) ? firstSplitL : (firstSplitL + 1))};
    std::cout << std::string(firstSplitL, '-') << ' ' << engineName << " status " << std::string(firstSplitR, '-') << '\n'
              << "Initial seed: " << theSeed << '\n'
              << "Current state: " << fPRBG << '\n'
              << split << std::endl;
}

template<Math::Random::UniformPseudoRandomBitGenerator PRBG>
auto Wrap<PRBG>::put(std::ostream& os) const -> decltype(os) {
    const auto engineName = name();
    os << engineName << "-begin\n"
       << fPRBG << '\n'
       << engineName << "-end\n";
    return os;
}

template<Math::Random::UniformPseudoRandomBitGenerator PRBG>
auto Wrap<PRBG>::get(std::istream& is) -> decltype(is) {
    const auto engineName = name();
    std::string tag;
    is >> tag;
    if (tag != std::string(engineName).append("-begin")) {
        std::cerr << "Wrap<PRBG>::get (with PRBG = " << muc::try_demangle(typeid(PRBG).name()) << "): No " << engineName << " found at current position, engine state unchanged. Input stream has been consumed\n";
        return is;
    }
    PRBG volunteer;
    is >> volunteer >> tag;
    if (tag != std::string(engineName).append("-end")) {
        std::cerr << "Wrap<PRBG>::get (with PRBG = " << muc::try_demangle(typeid(PRBG).name()) << "): " << engineName << " read from the input stream is incomplete, engine state unchanged. Input stream has been consumed\n";
        return is;
    }
    fPRBG = volunteer;
    return is;
}

template<Math::Random::UniformPseudoRandomBitGenerator PRBG>
auto Wrap<PRBG>::getState(std::istream& is) -> decltype(is) {
    PrintError("Wrap<PRBG>::getState has no effect. Do not use");
    return is;
}

} // namespace Mustard::CLHEPX::Random
