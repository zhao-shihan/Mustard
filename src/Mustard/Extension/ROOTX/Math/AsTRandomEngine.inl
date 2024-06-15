namespace Mustard::inline Extension::ROOTX::Math {

template<Mustard::Math::Random::UniformPseudoRandomBitGenerator PRBG>
AsTRandomEngine<PRBG>::AsTRandomEngine(typename PRBG::SeedType seed) :
    fPRBG{seed} {}

} // namespace Mustard::inline Extension::ROOTX::Math
