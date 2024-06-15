namespace Mustard::Math::Random::inline Generator {

constexpr Xoshiro256Plus::Xoshiro256Plus(Xoshiro256Plus::SeedType seed) :
    Xoshiro256Base{seed} {}

MUSTARD_ALWAYS_INLINE constexpr auto Xoshiro256Plus::operator()() -> Xoshiro256Plus::ResultType {
    const auto result{fState[0] + fState[3]};
    Step();
    return result;
}

} // namespace Mustard::Math::Random::inline Generator
