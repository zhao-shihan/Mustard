// -*- C++ -*-
//
// Copyright 2020-2025  The Mustard development team
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

#include <filesystem>

namespace Mustard::Parallel {

/// @brief Generates a unique filesystem path for parallel processing environments
///
/// This function creates a process-specific path to avoid file conflicts in MPI environments.
/// In single-process execution, returns the original path unchanged. In multi-process environments:
///   - On clusters: Creates node-specific subdirectories
///   - For all ranks: Appends MPI rank to filename stem
/// Directory creation is synchronized using intra-node MPI communication.
///
/// @param path Original filesystem path to process
///
/// @return The original path in single-process mode, or a modified path with:
///          - Node-specific parent directory (cluster environments)
///          - Filename stem appended with "_mpi{RANK}."
///          - Original extension preserved
///
/// @throws std::invalid_argument If:
///         - The path stem is empty
///         - The path stem is "." or ".."
/// @throws std::filesystem::filesystem_error On directory creation failures
///
/// @note Behavior depends on MPI configuration:
///       - Single process: Returns original path immediately
///       - Multi-process:
///          1. Checks filename validity
///          2. Creates node-specific directory (rank 0 per node)
///          3. Synchronizes directory creation (intra-node barrier)
///          4. Generates rank-unique filename
///       - Cluster environments use node names in directory structure
auto ProcessSpecificPath(const std::filesystem::path& path) -> std::filesystem::path;

} // namespace Mustard::Parallel
