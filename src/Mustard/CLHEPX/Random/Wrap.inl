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

namespace Mustard::CLHEPX::Random {

template<muc::random_number_engine PRNG>
Wrap<PRNG>::Wrap(long seed) :
    HepRandomEngine{},
    fPRNG{static_cast<typename PRNG::SeedType>(seed)} {
    theSeed = seed;
    theSeeds = &theSeed;
}

template<muc::random_number_engine PRNG>
auto Wrap<PRNG>::flatArray(const int size, double* vect) -> void {
    for (int i = 0; i < size; ++i) {
        vect[i] = Wrap::flat();
    }
}

template<muc::random_number_engine PRNG>
auto Wrap<PRNG>::setSeed(long seed, int) -> void {
    fPRNG.seed(seed);
    theSeed = seed;
    theSeeds = &theSeed;
}

template<muc::random_number_engine PRNG>
auto Wrap<PRNG>::setSeeds(const long* seeds, int) -> void {
    fPRNG.seed(*seeds);
    theSeed = *seeds;
    theSeeds = seeds;
}

template<muc::random_number_engine PRNG>
auto Wrap<PRNG>::saveStatus(gsl::czstring filename) const -> void {
    ProcessSpecificFile<std::ofstream> os{filename};
    if (not os.Opened()) [[unlikely]] {
        PrintError(fmt::format("Cannot open '{}', nothing was done (Wrap<PRNG>::name(): {}).",
                               os.Path(), name()));
    }
    put(os);
}

template<muc::random_number_engine PRNG>
auto Wrap<PRNG>::restoreStatus(gsl::czstring filename) -> void {
    ProcessSpecificFile<std::ifstream> is{filename};
    if (not is.Opened()) [[unlikely]] {
        PrintError(fmt::format("Cannot open '{}', nothing was done (Wrap<PRNG>::name(): {}).",
                               is.Path(), name()));
    }
    get(is);
}

template<muc::random_number_engine PRNG>
auto Wrap<PRNG>::showStatus() const -> void {
    constexpr std::string_view split{"----------------------------------------------------------------"};
    const auto engineName{name()};
    const auto firstSplit{static_cast<gsl::index>(split.length()) -          // length(split) -
                          static_cast<gsl::index>(engineName.length() + 9)}; // length(" " + engineName + " status ")
    const auto firstSplitL{std::max<gsl::index>(3, firstSplit / 2)};
    const auto firstSplitR{std::max<gsl::index>(3, muc::even(firstSplit) ? firstSplitL : (firstSplitL + 1))};
    std::cout << std::string(firstSplitL, '-') << ' ' << engineName << " status " << std::string(firstSplitR, '-') << '\n'
              << "Initial seed: " << theSeed << '\n'
              << "Current state: " << fPRNG << '\n'
              << split << std::endl;
}

template<muc::random_number_engine PRNG>
auto Wrap<PRNG>::put(std::ostream& os) const -> decltype(os) {
    const auto engineName = name();
    os << engineName << "-begin\n"
       << fPRNG << '\n'
       << engineName << "-end\n";
    return os;
}

template<muc::random_number_engine PRNG>
auto Wrap<PRNG>::get(std::istream& is) -> decltype(is) {
    const auto engineName = name();
    std::string tag;
    is >> tag;
    if (tag != std::string(engineName).append("-begin")) {
        std::cerr << "Wrap<PRNG>::get (with PRNG = " << muc::try_demangle(typeid(PRNG).name()) << "): No " << engineName << " found at current position, engine state unchanged. Input stream has been consumed\n";
        return is;
    }
    PRNG volunteer;
    is >> volunteer >> tag;
    if (tag != std::string(engineName).append("-end")) {
        std::cerr << "Wrap<PRNG>::get (with PRNG = " << muc::try_demangle(typeid(PRNG).name()) << "): " << engineName << " read from the input stream is incomplete, engine state unchanged. Input stream has been consumed\n";
        return is;
    }
    fPRNG = volunteer;
    return is;
}

template<muc::random_number_engine PRNG>
auto Wrap<PRNG>::getState(std::istream& is) -> decltype(is) {
    PrintError("Wrap<PRNG>::getState has no effect. Do not use.");
    return is;
}

} // namespace Mustard::CLHEPX::Random
