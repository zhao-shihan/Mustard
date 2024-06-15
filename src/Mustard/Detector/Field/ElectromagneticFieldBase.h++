#pragma once

#include "Mustard/Concept/NumericVector.h++"
#include "Mustard/Detector/Field/ElectromagneticField.h++"

#include "muc/array"

#include <concepts>

namespace Mustard::Detector::Field {

namespace internal {

template<Concept::NumericVector3D T>
struct BEFieldValue {
    T B;
    T E;
};

} // namespace internal

template<typename ADerived>
class ElectromagneticFieldBase {
public:
    template<Concept::NumericVector3D T>
    using F = internal::BEFieldValue<T>;

protected:
    constexpr ElectromagneticFieldBase();
    constexpr ~ElectromagneticFieldBase() = default;
};

} // namespace Mustard::Detector::Field

#include "Mustard/Detector/Field/ElectromagneticFieldBase.inl"
