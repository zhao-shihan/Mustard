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

#include <filesystem>

namespace Mustard::inline Extension::MPIX {

/// Create directories and file paths to help managing files during mpi processing.
///
/// For example:
///
/// An executable xxx do something like this:
///
///   MPI_Init(argc, argv);
///     ...
///   auto filePath = Mustard::MPIX::ParallelizePath("result.root");
///   // or, equivalently
///   // auto filePath = Mustard::MPIX::ParallelizePath("result", ".root");
///   SomeFileHandler::Open(filePath);
///     ...
///
/// When running xxx on 16 nodes (e.g. named as node0, ..., node15), each node has 24 processes:
/// A directory named result will be created, under which a series of directories named
/// node0, node1, ..., node15 will be created. And there will be a series of files named
/// result.mpi0.root, ..., result.mpi23.root under result/node0, etc. And a dirmap saves paths.
/// Directory structure as:
///
/// result ┬ node0 ┬ result.mpi0.root
///        │       ├ result.mpi1.root
///        │       ├ ...
///        │       └ result.mpi23.root
///        ├ node1 ┬ result.mpi24.root
///        │       ├ result.mpi25.root
///        │       ├ ...
///        │       └ result.mpi47.root
///        ├ ...
///        └ node15 ┬ result.mpi360.root
///                 ├ result.mpi361.root
///                 ├ ...
///                 └ result.mpi383.root
///
/// When running xxx on 1 node (e.g. PC), which has 8 processes:
/// A directory named result will be created, under which a series of files named
/// result.mpi0.root, ..., result.mpi7.root will be created. And a dirmap saves directory path.
/// Directory structure as:
///
/// result ┬ result.mpi0.root
///        ├ result.mpi1.root
///        ├ ...
///        └ result.mpi7.root
///
/// When just ./xxx (not in MPI mode) :
/// Just a single result.root will be created.
///
auto ParallelizePath(const std::filesystem::path& path) -> std::filesystem::path;

} // namespace Mustard::inline Extension::MPIX
