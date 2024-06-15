namespace Mustard::Math::Random::inline Distribution {

namespace internal {

template<Concept::Arithmetic T, template<typename> typename AUniform>
constexpr BasicUniformParameter<T, AUniform>::BasicUniformParameter() :
    BasicUniformParameter{0,
                          std::integral<T> ?
                              std::numeric_limits<T>::max() :
                              1} {}

template<Concept::Arithmetic T, template<typename> typename AUniform>
constexpr BasicUniformParameter<T, AUniform>::BasicUniformParameter(T inf, T sup) :
    DistributionParameterBase<BasicUniformParameter<T, AUniform>, AUniform<T>>(),
    fInfimum{inf},
    fSupremum{sup} {}

template<Concept::Arithmetic T, template<typename> typename AUniform>
template<Concept::Character AChar>
auto BasicUniformParameter<T, AUniform>::StreamOutput(std::basic_ostream<AChar>& os) const -> decltype(os) {
    if constexpr (std::floating_point<T>) {
        const auto oldPrecision{os.precision(std::numeric_limits<T>::max_digits10)};
        return os << fInfimum << ' ' << fSupremum
                  << std::setprecision(oldPrecision);
    } else {
        return os << fInfimum << ' ' << fSupremum;
    }
}

template<Concept::Arithmetic T, template<typename> typename AUniform>
template<Concept::Character AChar>
auto BasicUniformParameter<T, AUniform>::StreamInput(std::basic_istream<AChar>& is) & -> decltype(is) {
    return is >> fInfimum >> fSupremum;
}

template<template<typename> typename ADerived, Concept::Arithmetic T>
constexpr UniformBase<ADerived, T>::UniformBase(T inf, T sup) :
    Base{},
    fParameter{inf, sup} {}

template<template<typename> typename ADerived, Concept::Arithmetic T>
constexpr UniformBase<ADerived, T>::UniformBase(const typename Base::ParameterType& p) :
    Base{},
    fParameter{p} {}

} // namespace internal

template<std::floating_point T>
MUSTARD_ALWAYS_INLINE constexpr auto UniformCompact<T>::operator()(UniformRandomBitGenerator auto& g, const UniformCompactParameter<T>& p) -> T {
    const auto u{static_cast<T>(g() - g.Min()) / (g.Max() - g.Min())};
    muc::assume(0 <= u and u <= 1);
    return p.Infimum() + u * (p.Supremum() - p.Infimum());
}

template<std::floating_point T>
MUSTARD_ALWAYS_INLINE constexpr auto UniformReal<T>::operator()(UniformRandomBitGenerator auto& g, const UniformParameter<T>& p) -> T {
    T u;
    do {
        static_assert(UniformCompact<T>::Stateless());
        u = UniformCompact<T>{}(g);
        muc::assume(0 <= u and u <= 1);
    } while (u == 0 or u == 1);
    return p.Infimum() + u * (p.Supremum() - p.Infimum());
}

template<std::integral T>
MUSTARD_ALWAYS_INLINE constexpr auto UniformInteger<T>::operator()(UniformRandomBitGenerator auto& g, const UniformParameter<T>& p) -> T {
    // Do we need an independent implementation?
    return std::uniform_int_distribution<T>{p.Infimum(), p.Supremum()}(g);
}

} // namespace Mustard::Math::Random::inline Distribution
