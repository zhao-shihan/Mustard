// -*- C++ -*-
//
// Copyright (C) 2020-2025  The Mustard development team
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

template<typename...>
class DetectorDescriptionModule;

template<>
class DetectorDescriptionModule<> : public ModuleBase {
public:
    DetectorDescriptionModule(gsl::not_null<CLI<>*> cli);
    virtual ~DetectorDescriptionModule() = default;

    virtual auto DetectorDescriptionIOIfFlagged() const -> void = 0;
};

template<muc::tuple_like ADescriptionTuple>
class DetectorDescriptionModule<ADescriptionTuple> : public DetectorDescriptionModule<> {
public:
    using DetectorDescriptionModule<>::DetectorDescriptionModule;

    virtual auto DetectorDescriptionIOIfFlagged() const -> void override;
};

template<Detector::Description::Description... Ds>
class DetectorDescriptionModule<Ds...> : public DetectorDescriptionModule<std::tuple<Ds...>> {
public:
    using DetectorDescriptionModule<std::tuple<Ds...>>::DetectorDescriptionModule;
};

} // namespace Mustard::CLI::inline Module

#include "Mustard/CLI/Module/DetectorDescriptionModule.inl"
