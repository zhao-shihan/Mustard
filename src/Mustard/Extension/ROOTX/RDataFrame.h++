// -*- C++ -*-
//
// Copyright 2020-2024  The Mustard development team
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

#include "ROOT/RDF/InterfaceUtils.hxx"

#include <concepts>

namespace Mustard::inline Extension::ROOTX {

template<typename T>
concept RDataFrame [[deprecated("Use ROOT::RDF::RNode instead")]] =
    requires {
        requires std::convertible_to<T, ROOT::RDF::RNode>;
        requires not std::is_const_v<std::remove_reference_t<T>>;
    };

} // namespace Mustard::inline Extension::ROOTX
