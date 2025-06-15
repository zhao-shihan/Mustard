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

#include "Mustard/Extension/MPIX/DataType.h++"
#include "Mustard/Utility/PrettyLog.h++"

#include "ROOT/RDataFrame.hxx"

#include "mpl/mpl.hpp"

#include "muc/algorithm"
#include "muc/hash_map"
#include "muc/hash_set"

#include "gsl/gsl"

#include "fmt/core.h"

#include <algorithm>
#include <array>
#include <concepts>
#include <limits>
#include <ranges>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace Mustard::Data {

template<std::integral T>
auto RDFEventSplit(ROOT::RDF::RNode rdf,
                   std::string eventIDColumnName) -> std::vector<gsl::index>;

struct RDFEntryRange {
    gsl::index first;
    gsl::index last;
};

template<std::integral T, std::size_t N>
auto RDFEventSplit(std::array<ROOT::RDF::RNode, N> rdf,
                   const std::string& eventIDColumnName) -> std::vector<std::array<RDFEntryRange, N>>;

template<std::integral T, std::size_t N>
auto RDFEventSplit(std::array<ROOT::RDF::RNode, N> rdf,
                   const std::array<std::string, N>& eventIDColumnName) -> std::vector<std::array<RDFEntryRange, N>>;

} // namespace Mustard::Data

#include "Mustard/Data/RDFEventSplit.inl"
