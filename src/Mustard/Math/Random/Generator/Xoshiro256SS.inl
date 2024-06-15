namespace Mustard::Math::Random::inline Generator {

constexpr Xoshiro256SS::Xoshiro256SS(Xoshiro256SS::SeedType seed) :
    Xoshiro256Base{seed} {}

MUSTARD_ALWAYS_INLINE constexpr auto Xoshiro256SS::operator()() -> Xoshiro256SS::ResultType {
    const auto result{std::rotl(fState[1] * 5, 7) * 9};
    Step();
    return result;
}

} // namespace Mustard::Math::Random::inline Generator
