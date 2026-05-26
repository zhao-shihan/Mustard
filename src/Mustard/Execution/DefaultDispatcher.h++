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

auto DefaultDispatcherCode() -> std::string;

template<std::integral T>
auto MakeCodedDispatcher(std::string_view dispatcher) -> std::unique_ptr<Dispatcher<T>>;

template<std::integral T>
auto MakeDefaultDispatcher() -> std::unique_ptr<Dispatcher<T>>;

} // namespace Mustard::inline Execution

#include "Mustard/Execution/DefaultDispatcher.inl"
