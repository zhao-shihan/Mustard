#pragma once

#include "Mustard/Concept/NumericVector.h++"
#include "Mustard/Detector/Field/ElectricField.h++"
#include "Mustard/Detector/Field/ElectromagneticFieldBase.h++"
#include "Mustard/Detector/Field/MagneticFieldBase.h++"

#include <concepts>

namespace Mustard::Detector::Field {

template<typename ADerived>
class MagneticFieldBase;

template<typename ADerived>
class ElectricFieldBase : public ElectromagneticFieldBase<ADerived> {
public:
    template<Concept::NumericVector3D T>
    using F = typename ElectromagneticFieldBase<ADerived>::template F<T>;

protected:
    constexpr ElectricFieldBase();
    constexpr ~ElectricFieldBase() = default;

public:
    template<Concept::NumericVector3D T>
    static constexpr auto B(T) -> T { return {0, 0, 0}; }
    template<Concept::NumericVector3D T>
    constexpr auto BE(T x) const -> F<T> { return {B(x), static_cast<const ADerived*>(this)->E(x)}; }
};

} // namespace Mustard::Detector::Field

#include "Mustard/Detector/Field/ElectricFieldBase.inl"
