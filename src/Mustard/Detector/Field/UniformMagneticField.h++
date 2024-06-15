#pragma once

#include "Mustard/Concept/InputVector.h++"
#include "Mustard/Concept/NumericVector.h++"
#include "Mustard/Detector/Field/MagneticFieldBase.h++"

#include "muc/array"

namespace Mustard::Detector::Field {

class UniformMagneticField : public MagneticFieldBase<UniformMagneticField> {
public:
    constexpr UniformMagneticField(double bx, double by, double bz);
    template<Concept::InputVector3D T = muc::array3d>
    constexpr UniformMagneticField(T b);

    template<Concept::NumericVector3D T>
    constexpr auto B(T) const -> T { return {fBx, fBy, fBz}; }

private:
    double fBx;
    double fBy;
    double fBz;
};

} // namespace Mustard::Detector::Field

#include "Mustard/Detector/Field/UniformMagneticField.inl"
