#pragma once

#include "Mustard/Concept/NumericVector.h++"

#include "muc/array"

#include <concepts>

namespace Mustard::Detector::Field {

template<typename F>
concept ElectromagneticField =
    requires(const F f, muc::array3d x) {
        { f.B(x) } -> std::same_as<muc::array3d>;
        { f.E(x) } -> std::same_as<muc::array3d>;
        { f.BE(x).B } -> std::same_as<muc::array3d&&>;
        { f.BE(x).E } -> std::same_as<muc::array3d&&>;
        { f.template B<muc::array3d>({}) } -> std::same_as<muc::array3d>;
        { f.template E<muc::array3d>({}) } -> std::same_as<muc::array3d>;
        { f.template BE<muc::array3d>({}).B } -> std::same_as<muc::array3d&&>;
        { f.template BE<muc::array3d>({}).E } -> std::same_as<muc::array3d&&>;
    };

} // namespace Mustard::Detector::Field
