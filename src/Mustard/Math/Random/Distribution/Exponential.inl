namespace Mustard::Math::Random::inline Distribution {

namespace internal {

template<std::floating_point T, template<typename> typename AExponential>
constexpr BasicExponentialParameter<T, AExponential>::BasicExponentialParameter() :
    BasicExponentialParameter{1} {}

template<std::floating_point T, template<typename> typename AExponential>
constexpr BasicExponentialParameter<T, AExponential>::BasicExponentialParameter(T expectation) :
    Base{},
    fExpectation{expectation} {}

template<std::floating_point T, template<typename> typename AExponential>
template<Concept::Character AChar>
auto BasicExponentialParameter<T, AExponential>::StreamOutput(std::basic_ostream<AChar>& os) const -> decltype(os) {
    const auto oldPrecision{os.precision(std::numeric_limits<T>::max_digits10)};
    return os << fExpectation
              << std::setprecision(oldPrecision);
}

template<std::floating_point T, template<typename> typename AExponential>
template<Concept::Character AChar>
auto BasicExponentialParameter<T, AExponential>::StreamInput(std::basic_istream<AChar>& is) & -> decltype(is) {
    return is >> fExpectation;
}

template<template<typename> typename ADerived, std::floating_point T>
constexpr ExponentialBase<ADerived, T>::ExponentialBase(T expectation) :
    Base{},
    fParameter{expectation} {}

template<template<typename> typename ADerived, std::floating_point T>
constexpr ExponentialBase<ADerived, T>::ExponentialBase(const typename Base::ParameterType& p) :
    Base{},
    fParameter{p} {}

} // namespace internal

template<std::floating_point T>
MUSTARD_STRONG_INLINE constexpr auto Exponential<T>::operator()(UniformRandomBitGenerator auto& g, const ExponentialParameter<T>& p) -> T {
    static_assert(Uniform<T>::Stateless());
    return -p.Expectation() * std::log(Uniform<T>{}(g));
}

template<std::floating_point T>
MUSTARD_ALWAYS_INLINE constexpr auto ExponentialFast<T>::operator()(UniformRandomBitGenerator auto& g, const ExponentialFastParameter<T>& p) -> T {
    static_assert(Uniform<T>::Stateless());
    return -p.Expectation() * internal::FastLogForOpen01(Uniform<T>{}(g));
}

} // namespace Mustard::Math::Random::inline Distribution
