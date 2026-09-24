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

template<Math::Random::RandomNumberEngineOrReference R>
AsHepRandomEngine<R>::AsHepRandomEngine()
    requires(not std::is_reference_v<R>)
    :
    CLHEP::HepRandomEngine{},
    Base{},
    NoncopyableBase{} {
    RecordSeed(0); // The engine is at its default state, so its seed is unknown.
}

template<Math::Random::RandomNumberEngineOrReference R>
AsHepRandomEngine<R>::AsHepRandomEngine(SeedType seed)
    requires(not std::is_reference_v<R>)
    :
    CLHEP::HepRandomEngine{},
    Base{seed},
    NoncopyableBase{} {
    RecordSeed(static_cast<long>(seed));
}

template<Math::Random::RandomNumberEngineOrReference R>
AsHepRandomEngine<R>::AsHepRandomEngine(EngineType& engine)
    requires(std::is_reference_v<R>)
    :
    CLHEP::HepRandomEngine{},
    Base{engine},
    NoncopyableBase{} {
    RecordSeed(0); // The engine belongs to someone else, so its seed is unknown.
}

template<Math::Random::RandomNumberEngineOrReference R>
auto AsHepRandomEngine<R>::flatArray(const int size, double* vect) -> void {
    for (int i = 0; i < size; ++i) {
        vect[i] = AsHepRandomEngine::flat();
    }
}

template<Math::Random::RandomNumberEngineOrReference R>
auto AsHepRandomEngine<R>::setSeed(long seed, int) -> void {
    Engine().seed(static_cast<SeedType>(seed));
    RecordSeed(seed);
}

template<Math::Random::RandomNumberEngineOrReference R>
auto AsHepRandomEngine<R>::setSeeds(const long* seeds, int) -> void {
    if (seeds == nullptr) { // CLHEP's own engines tolerate a null seed array
        Engine().seed();
        RecordSeed(0); // back to the default state, so the seed is unknown again
        return;
    }
    Engine().seed(static_cast<SeedType>(*seeds));
    RecordSeed(*seeds);
}

template<Math::Random::RandomNumberEngineOrReference R>
auto AsHepRandomEngine<R>::saveStatus(gsl::czstring filename) const -> void {
    ProcessSpecificFile<std::ofstream> os{filename};
    if (not os.Opened()) [[unlikely]] {
        PrintError(fmt::format("Cannot open '{}', nothing was done (AsHepRandomEngine<R>::name(): {}).",
                               os.Path(), name()));
    }
    put(os);
}

template<Math::Random::RandomNumberEngineOrReference R>
auto AsHepRandomEngine<R>::restoreStatus(gsl::czstring filename) -> void {
    ProcessSpecificFile<std::ifstream> is{filename};
    if (not is.Opened()) [[unlikely]] {
        PrintError(fmt::format("Cannot open '{}', nothing was done (AsHepRandomEngine<R>::name(): {}).",
                               is.Path(), name()));
    }
    get(is);
}

template<Math::Random::RandomNumberEngineOrReference R>
auto AsHepRandomEngine<R>::showStatus() const -> void {
    constexpr std::string_view split{"----------------------------------------------------------------"};
    const auto engineName{name()};
    const auto firstSplit{static_cast<gsl::index>(split.length()) -          // length(split) -
                          static_cast<gsl::index>(engineName.length() + 9)}; // length(" " + engineName + " status ")
    const auto firstSplitL{std::max<gsl::index>(3, firstSplit / 2)};
    const auto firstSplitR{std::max<gsl::index>(3, muc::even(firstSplit) ? firstSplitL : (firstSplitL + 1))};
    fmt::println("{} {} status {}\nInitial seed: {}\nCurrent state: {}\n{}",
                 std::string(firstSplitL, '-'), engineName, std::string(firstSplitR, '-'),
                 theSeed, fmt::streamed(Engine()), split);
}

template<Math::Random::RandomNumberEngineOrReference R>
auto AsHepRandomEngine<R>::put(std::ostream& os) const -> decltype(os) {
    const auto engineName = name();
    os << engineName << "-begin\n"
       << Engine() << '\n'
       << engineName << "-end\n";
    return os;
}

template<Math::Random::RandomNumberEngineOrReference R>
auto AsHepRandomEngine<R>::get(std::istream& is) -> decltype(is) {
    const auto engineName{name()};
    // An engine name may contain whitespace, since a type-erased engine names
    // its template arguments, so the tags are read line by line, not as tokens
    std::string tag;
    std::getline(is, tag);
    if (tag != std::string(engineName).append("-begin")) {
        PrintError(fmt::format("CLHEPX::Random::AsHepRandomEngine<R>::get (with R = {}): No {} found at current position, engine state unchanged. Input stream has been consumed.",
                               muc::try_demangle(typeid(AsHepRandomEngine).name()), engineName));
        return is;
    }
    // The state is restored into an engine of the installed type, which a
    // default constructed one need not be: an engine may erase its type
    EngineType volunteer{Engine()};
    is >> volunteer >> std::ws;
    std::getline(is, tag);
    if (tag != std::string(engineName).append("-end")) {
        PrintError(fmt::format("CLHEPX::Random::AsHepRandomEngine<R>::get (with R = {}): {} read from the input stream is incomplete, engine state unchanged. Input stream has been consumed.",
                               muc::try_demangle(typeid(AsHepRandomEngine).name()), engineName));
        return is;
    }
    Engine() = volunteer;
    return is;
}

template<Math::Random::RandomNumberEngineOrReference R>
template<std::floating_point T>
auto AsHepRandomEngine<R>::DrawOpen01() -> T {
    T value;
    do {
        value = Math::Random::Uniform<T>{}(Engine());
    } while (value == 0); // CLHEP excludes the end points of ]0, 1[
    return value;
}

template<Math::Random::RandomNumberEngineOrReference R>
auto AsHepRandomEngine<R>::getState(std::istream& is) -> decltype(is) {
    PrintError("CLHEPX::Random::AsHepRandomEngine<R>::getState has no effect. Do not use.");
    return is;
}

template<Math::Random::RandomNumberEngineOrReference R>
auto AsHepRandomEngine<R>::RecordSeed(long seed) -> void {
    theSeed = seed;
    fSeedArray[0] = seed;
    fSeedArray[1] = 0; // CLHEP's seed array is zero-terminated
    theSeeds = fSeedArray;
}

} // namespace Mustard::CLHEPX::Random
