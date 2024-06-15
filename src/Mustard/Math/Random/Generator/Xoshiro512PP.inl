namespace Mustard::Math::Random::inline Generator {

constexpr Xoshiro512PP::Xoshiro512PP(Xoshiro512PP::SeedType seed) :
    Xoshiro512Base{seed} {}

MUSTARD_ALWAYS_INLINE constexpr auto Xoshiro512PP::operator()() -> Xoshiro512PP::ResultType {
    const auto result{std::rotl(fState[0] + fState[2], 17) + fState[2]};
    Step();
    return result;
}

} // namespace Mustard::Math::Random::inline Generator
