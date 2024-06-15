namespace Mustard::Math::Random {

template<typename ADerived,
         std::unsigned_integral AResult>
constexpr UniformRandomBitGeneratorBase<ADerived,
                                        AResult>::UniformRandomBitGeneratorBase() {
    static_assert(UniformRandomBitGenerator<ADerived>);
}

} // namespace Mustard::Math::Random
