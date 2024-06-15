namespace Mustard::inline Extension::ROOTX::Math {

template<Mustard::Math::Random::UniformPseudoRandomBitGenerator PRBG>
AsTRandom<PRBG>::AsTRandom(typename PRBG::SeedType seed) :
    fPRBG{seed},
    fGaussian{} {}

template<Mustard::Math::Random::UniformPseudoRandomBitGenerator PRBG>
auto AsTRandom<PRBG>::RndmArray(Int_t n, Float_t* array) -> void {
    std::ranges::generate_n(array, n, [this] { return Mustard::Math::Random::Uniform<Float_t>{}(fPRBG); });
}

template<Mustard::Math::Random::UniformPseudoRandomBitGenerator PRBG>
auto AsTRandom<PRBG>::RndmArray(Int_t n, Double_t* array) -> void {
    std::ranges::generate_n(array, n, [this] { return Rndm(); });
}

template<Mustard::Math::Random::UniformPseudoRandomBitGenerator PRBG>
auto AsTRandom<PRBG>::GetSeed() const -> UInt_t {
    Env::PrintLnError("AsTRandom<PRBG>::GetSeed has no effect. Do not use");
    return 0;
}

} // namespace Mustard::inline Extension::ROOTX::Math
