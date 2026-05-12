// -*- C++ -*-
//
// Copyright (C) 2020-2026  Shihan Zhao and contributors
//
// This file is part of Mustard, an offline software framework for HEP experiments.
//
// Mustard is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// Mustard is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// Mustard. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "Mustard/CLI/CLI.h++"
#include "Mustard/CLI/Module/ModuleBase.h++"
#include "Mustard/Detector/Description/Description.h++"
#include "Mustard/Detector/Description/DescriptionIO.h++"

#include "G4VUserDetectorConstruction.hh"

#include "muc/tuple"

#include "gsl/gsl"

#include <concepts>
#include <tuple>

namespace Mustard::CLI::inline Module {

/// @brief Variadic module template for detector description import/export.
///
/// DetectorDescriptionModule handles the --import-dd, --export-dd, and
/// --emport-dd CLI flags and delegates to Detector::Description::DescriptionIO
/// for the actual I/O operations.
///
/// Three specializations:
/// - DetectorDescriptionModule<> : abstract base declaring the virtual I/O method.
/// - DetectorDescriptionModule<ADescriptionTuple> : tuple-based partial specialization.
/// - DetectorDescriptionModule<Ds...> : variadic specialization for concrete Description types.
template<typename...>
class DetectorDescriptionModule;

/// @brief Base specialization: declares the pure virtual I/O method.
///
/// Registers the --import-dd, --export-dd, and --emport-dd arguments.
/// Derived classes must override DetectorDescriptionIOIfFlagged().
template<>
class DetectorDescriptionModule<> : public ModuleBase {
public:
    /// @brief Construct and register detector description I/O arguments.
    /// @param cli  Owning CLI instance.
    DetectorDescriptionModule(gsl::not_null<CLI<>*> cli);

    /// @brief Virtual destructor.
    virtual ~DetectorDescriptionModule() = default;

    /// @brief Execute detector description I/O if a relevant flag was used.
    ///
    /// Implementations should check --import-dd, --export-dd, and --emport-dd
    /// and delegate to the appropriate Detector::Description::DescriptionIO method.
    virtual auto DetectorDescriptionIOIfFlagged() const -> void = 0;
};

/// @brief Tuple-based partial specialization.
///
/// Implements DetectorDescriptionIOIfFlagged() for a muc::tuple_like
/// description tuple by dispatching to DescriptionIO.
/// @tparam ADescriptionTuple  A type satisfying muc::tuple_like.
template<muc::tuple_like ADescriptionTuple>
class DetectorDescriptionModule<ADescriptionTuple> : public DetectorDescriptionModule<> {
public:
    using DetectorDescriptionModule<>::DetectorDescriptionModule;

    /// @brief Perform I/O for the tuple-based description types.
    virtual auto DetectorDescriptionIOIfFlagged() const -> void override;
};

/// @brief Convenience variadic specialization for concrete Description types.
///
/// Accepts a pack of Detector::Description::Description types and
/// wraps them in a std::tuple.
/// @tparam Ds  Pack of Detector::Description::Description types.
template<Detector::Description::Description... Ds>
class DetectorDescriptionModule<Ds...> : public DetectorDescriptionModule<std::tuple<Ds...>> {
public:
    using DetectorDescriptionModule<std::tuple<Ds...>>::DetectorDescriptionModule;
};

} // namespace Mustard::CLI::inline Module

#include "Mustard/CLI/Module/DetectorDescriptionModule.inl"
