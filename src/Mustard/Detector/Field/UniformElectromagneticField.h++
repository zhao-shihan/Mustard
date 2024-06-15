#pragma once

#include "Mustard/Concept/InputVector.h++"
#include "Mustard/Concept/NumericVector.h++"
#include "Mustard/Detector/Field/ElectromagneticFieldBase.h++"

#include "muc/array"

namespace Mustard::Detector::Field {

class UniformElectromagneticField : public ElectromagneticFieldBase<UniformElectromagneticField> {
public:
    constexpr UniformElectromagneticField(double bx, double by, double bz,
                                          double ex, double ey, double ez);
    template<Concept::InputVector3D T1 = muc::array3d, Concept::InputVector3D T2 = muc::array3d>
    constexpr UniformElectromagneticField(T1 b, T2 e);

    template<Concept::NumericVector3D T>
    constexpr auto B(T) const -> T { return {fBx, fBy, fBz}; }
    template<Concept::NumericVector3D T>
    constexpr auto E(T) const -> T { return {fEx, fEy, fEz}; }
    template<Concept::NumericVector3D T>
    constexpr auto BE(T x) const -> F<T> { return {B(x), E(x)}; }

private:
    double fBx;
    double fBy;
    double fBz;
    double fEx;
    double fEy;
    double fEz;
};

} // namespace Mustard::Detector::Field

#include "Mustard/Detector/Field/UniformElectromagneticField.inl"
