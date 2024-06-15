namespace Mustard::Math::Random {

template<typename ADerived,
         std::unsigned_integral AResult,
         std::unsigned_integral ASeed>
constexpr UniformPseudoRandomBitGeneratorBase<ADerived,
                                              AResult,
                                              ASeed>::UniformPseudoRandomBitGeneratorBase() :
    UniformRandomBitGeneratorBase<ADerived, AResult>{} {
    static_assert(UniformPseudoRandomBitGenerator<ADerived>);
}

} // namespace Mustard::Math::Random
