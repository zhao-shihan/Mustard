namespace Mustard::Math::Random::inline Generator {

template<typename ADerived, std::size_t NBit>
    requires(NBit % 64 == 0)
constexpr XoshiroBase<ADerived, NBit>::XoshiroBase(std::array<std::uint64_t, NBit / 64> defaultState) :
    UniformPseudoRandomBitGeneratorBase<ADerived, std::uint64_t, std::uint64_t>{},
    fState{std::move(defaultState)} {
    static_assert(std::derived_from<ADerived, XoshiroBase<ADerived, NBit>>);
}

template<typename ADerived, std::size_t NBit>
    requires(NBit % 64 == 0)
constexpr XoshiroBase<ADerived, NBit>::XoshiroBase(std::uint64_t seed) :
    UniformPseudoRandomBitGeneratorBase<ADerived, std::uint64_t, std::uint64_t>{},
    fState{} {
    Seed(seed);
}

template<typename ADerived, std::size_t NBit>
    requires(NBit % 64 == 0)
constexpr auto XoshiroBase<ADerived, NBit>::Seed(std::uint64_t seed) -> void {
    SplitMix64 splitMix64{seed};
    std::ranges::generate(fState, splitMix64);
    static_cast<ADerived*>(this)->Step();
    static_cast<ADerived*>(this)->Step();
    static_cast<ADerived*>(this)->Step();
}

} // namespace Mustard::Math::Random::inline Generator
