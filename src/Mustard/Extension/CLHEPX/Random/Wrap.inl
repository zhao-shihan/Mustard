namespace Mustard::inline Extension::CLHEPX::Random {

template<Math::Random::UniformPseudoRandomBitGenerator PRBG>
Wrap<PRBG>::Wrap() :
    Wrap{20030202} {}

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
        Env::PrintLnError("Wrap<PRBG>::saveStatus: Cannot open '{}', nothing was done. (Wrap<PRBG>::name(): {})",
                          filename, name());
    }
}

template<Math::Random::UniformPseudoRandomBitGenerator PRBG>
auto Wrap<PRBG>::restoreStatus(gsl::czstring filename) -> void {
    std::ifstream is(filename, std::ios::in);
    if (is.is_open()) {
        get(is);
    } else {
        Env::PrintLnError("Wrap<PRBG>::restoreStatus: Cannot open '{}', nothing was done. (Wrap<PRBG>::name(): {})",
                          filename, name());
    }
}

template<Math::Random::UniformPseudoRandomBitGenerator PRBG>
auto Wrap<PRBG>::showStatus() const -> void {
    constexpr std::string_view split = "----------------------------------------------------------------";
    const auto engineName = name();
    const auto firstSplit = static_cast<std::intmax_t>(split.length()) -         // length(split) -
                            static_cast<std::intmax_t>(engineName.length() + 9); // length(" " + engineName + " status ")
    const auto firstSplitL = std::max<std::intmax_t>(3, firstSplit / 2);
    const auto firstSplitR = std::max<std::intmax_t>(3, Math::IsEven(firstSplit) ? firstSplitL : (firstSplitL + 1));
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
        std::cerr << "Wrap<PRBG>::get (with PRBG = " << typeid(PRBG).name() << "): No " << engineName << " found at current position, engine state unchanged. Input stream has been consumed\n";
        return is;
    }
    PRBG volunteer;
    is >> volunteer >> tag;
    if (tag != std::string(engineName).append("-end")) {
        std::cerr << "Wrap<PRBG>::get (with PRBG = " << typeid(PRBG).name() << "): " << engineName << " read from the input stream is incomplete, engine state unchanged. Input stream has been consumed\n";
        return is;
    }
    fPRBG = volunteer;
    return is;
}

template<Math::Random::UniformPseudoRandomBitGenerator PRBG>
auto Wrap<PRBG>::getState(std::istream& is) -> decltype(is) {
    Env::PrintLnError("Wrap<PRBG>::getState has no effect. Do not use");
    return is;
}

} // namespace Mustard::inline Extension::CLHEPX::Random
