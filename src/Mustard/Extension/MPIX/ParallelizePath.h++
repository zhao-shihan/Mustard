#pragma once

#include <concepts>
#include <filesystem>
#include <string_view>

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
