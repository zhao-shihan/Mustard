#pragma once

#include "Mustard/Concept/InputVector.h++"
#include "Mustard/Concept/NumericVector.h++"
#include "Mustard/Detector/Field/ElectricFieldBase.h++"

namespace Mustard::Detector::Field {

class UniformElectricField : public ElectricFieldBase<UniformElectricField> {
public:
    constexpr UniformElectricField(double ex, double ey, double ez);
    template<Concept::InputVector3D T = muc::array3d>
    constexpr UniformElectricField(T e);

    template<Concept::NumericVector3D T>
    constexpr auto E(T) const -> T { return {fEx, fEy, fEz}; }

private:
    double fEx;
    double fEy;
    double fEz;
};

} // namespace Mustard::Detector::Field

#include "Mustard/Detector/Field/UniformElectricField.inl"
