#pragma once

#include "Mustard/Concept/NumericVector.h++"
#include "Mustard/Detector/Field/ElectricFieldBase.h++"
#include "Mustard/Detector/Field/ElectromagneticFieldBase.h++"
#include "Mustard/Detector/Field/MagneticField.h++"

#include <concepts>

namespace Mustard::Detector::Field {

template<typename ADerived>
class ElectricFieldBase;

template<typename ADerived>
class MagneticFieldBase : public ElectromagneticFieldBase<ADerived> {
public:
    template<Concept::NumericVector3D T>
    using F = typename ElectromagneticFieldBase<ADerived>::template F<T>;

protected:
    constexpr MagneticFieldBase();
    constexpr ~MagneticFieldBase() = default;

public:
    template<Concept::NumericVector3D T>
    static constexpr auto E(T) -> T { return {0, 0, 0}; }
    template<Concept::NumericVector3D T>
    constexpr auto BE(T x) const -> F<T> { return {static_cast<const ADerived*>(this)->B(x), E(x)}; }
};

} // namespace Mustard::Detector::Field

#include "Mustard/Detector/Field/MagneticFieldBase.inl"
