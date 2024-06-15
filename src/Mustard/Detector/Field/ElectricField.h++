#pragma once

#include "Mustard/Detector/Field/ElectromagneticField.h++"

#include "muc/array"

#include <concepts>

namespace Mustard::Detector::Field {

template<typename F>
concept ElectricField =
    requires {
        requires ElectromagneticField<F>;
        { F::template B<muc::array3d>({}) } -> std::same_as<muc::array3d>;
        requires F::template B<muc::array3d>({}) == muc::array3d{};
    };

} // namespace Mustard::Detector::Field
