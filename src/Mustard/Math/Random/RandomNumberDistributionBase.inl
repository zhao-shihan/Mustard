namespace Mustard::Math::Random {

template<typename ADerived, typename ADistribution>
constexpr DistributionParameterBase<ADerived, ADistribution>::DistributionParameterBase() {
    static_assert(DistributionParameterOf<ADerived, ADistribution>);
}

template<typename ADerived, typename AParameter, typename T>
    requires(std::is_arithmetic_v<T> or Concept::NumericVectorAny<T>)
constexpr RandomNumberDistributionBase<ADerived, AParameter, T>::RandomNumberDistributionBase() {
    static_assert(RandomNumberDistribution<ADerived>);
}

} // namespace Mustard::Math::Random
