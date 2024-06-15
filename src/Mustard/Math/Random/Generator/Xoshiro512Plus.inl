namespace Mustard::Math::Random::inline Generator {

constexpr Xoshiro512Plus::Xoshiro512Plus(Xoshiro512Plus::SeedType seed) :
    Xoshiro512Base{seed} {}

MUSTARD_ALWAYS_INLINE constexpr auto Xoshiro512Plus::operator()() -> Xoshiro512Plus::ResultType {
    const auto result{fState[0] + fState[2]};
    Step();
    return result;
}

} // namespace Mustard::Math::Random::inline Generator
