// -*- C++ -*-
//
// Copyright (C) 2020-2025  Mustard developers
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

#include "ROOT/RDataFrame.hxx"

#include "gtl/vector.hpp"

namespace Mustard::Data::inline Processing {

/// @brief Count entries for a single ROOT RDataFrame node.
/// @param rdf Input ROOT RDataFrame node.
/// @return Number of entries.
/// @note Local path (`mplr::available()==false` or world size is 1): directly
///   returns `*rdf.Count()` in the local process.
/// @note MPI multi-process path (`mplr::available()==true` and world size > 1):
///   world rank 0 evaluates `*rdf.Count()`, then broadcasts the result.
auto CountRDFEntry(ROOT::RDF::RNode rdf) -> long long;

/// @brief Count entries for multiple ROOT RDataFrame nodes.
/// @param rdf Input ROOT RDataFrame nodes.
/// @return Entry counts in the same order as @p rdf.
/// @note Local path (`mplr::available()==false` or world size is 1): counts
///   each node with `std::async` in the local process.
/// @note MPI multi-process path (`mplr::available()==true` and world size > 1):
///   selected node leaders compute counts, then values are broadcast among node
///   leaders (inter-node), and finally broadcast to all local processes
///   (intra-node).
auto CountRDFEntry(std::vector<ROOT::RDF::RNode> rdf) -> gtl::vector<long long>;

} // namespace Mustard::Data::inline Processing
