namespace Mustard::Math::Random::inline Generator {

constexpr Xoshiro512SS::Xoshiro512SS(Xoshiro512SS::SeedType seed) :
    Xoshiro512Base{seed} {}

MUSTARD_ALWAYS_INLINE constexpr auto Xoshiro512SS::operator()() -> Xoshiro512SS::ResultType {
    const auto result{std::rotl(fState[1] * 5, 7) * 9};
    Step();
    return result;
}

} // namespace Mustard::Math::Random::inline Generator
