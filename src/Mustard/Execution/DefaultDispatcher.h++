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

#include "Mustard/Execution/ClusterAwareMasterWorkerDispatcher.h++"
#include "Mustard/Execution/Dispatcher.h++"
#include "Mustard/Execution/MasterWorkerDispatcher.h++"
#include "Mustard/Execution/SequentialDispatcher.h++"
#include "Mustard/Execution/SharedMemoryDispatcher.h++"
#include "Mustard/Execution/StaticDispatcher.h++"

#include "gtl/phmap.hpp"

#include "muc/algorithm"

#include "fmt/ranges.h"

#include <algorithm>
#include <concepts>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace Mustard::inline Execution {

/// @brief Return the default dispatcher code for the current environment.
///
/// The value of the @c MUSTARD_EXECUTION_DISPATCHER environment variable is
/// used when set. Otherwise the code is selected from the MPI configuration:
/// @c seq for a single process, @c shm for a single shared-memory domain,
/// @c mw for small worlds, and @c cmw for large multi-node clusters.
///
/// @return Dispatcher code
/// @see MakeCodedDispatcher
auto DefaultDispatcherCode() -> std::string;

/// @brief Construct a dispatcher from its code.
///
/// Supported codes are @c seq (sequential), @c stat (static rank-interleaved),
/// @c shm (shared-memory dynamic), @c mw (master-worker), and @c cmw
/// (cluster-aware master-worker).
///
/// @tparam T Task index type
/// @param dispatcher Dispatcher code
/// @return Newly constructed dispatcher
/// @throws std::out_of_range if the code is not recognized
template<std::integral T>
auto MakeCodedDispatcher(std::string_view dispatcher) -> std::unique_ptr<Dispatcher<T>>;

/// @brief Construct the dispatcher selected by DefaultDispatcherCode().
/// @tparam T Task index type
/// @return Newly constructed dispatcher
template<std::integral T>
auto MakeDefaultDispatcher() -> std::unique_ptr<Dispatcher<T>>;

} // namespace Mustard::inline Execution

#include "Mustard/Execution/DefaultDispatcher.inl"
