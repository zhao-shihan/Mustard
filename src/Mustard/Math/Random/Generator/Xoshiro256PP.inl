namespace Mustard::Math::Random::inline Generator {

constexpr Xoshiro256PP::Xoshiro256PP(Xoshiro256PP::SeedType seed) :
    Xoshiro256Base{seed} {}

MUSTARD_ALWAYS_INLINE constexpr auto Xoshiro256PP::operator()() -> Xoshiro256PP::ResultType {
    const auto result{std::rotl(fState[0] + fState[3], 23) + fState[0]};
    Step();
    return result;
}

} // namespace Mustard::Math::Random::inline Generator
