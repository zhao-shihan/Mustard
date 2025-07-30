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

#include "Mustard/Execution/ClusterAwareMasterWorkerScheduler.h++"
#include "Mustard/Execution/MasterWorkerScheduler.h++"
#include "Mustard/Execution/Scheduler.h++"
#include "Mustard/Execution/SequentialScheduler.h++"
#include "Mustard/Execution/SharedMemoryScheduler.h++"
#include "Mustard/Execution/StaticScheduler.h++"

#include "fmt/ranges.h"

#include <algorithm>
#include <concepts>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace Mustard::inline Execution {

auto DefaultSchedulerCode() -> std::string;

template<std::integral T>
auto MakeCodedScheduler(std::string_view scheduler) -> std::unique_ptr<Scheduler<T>>;

template<std::integral T>
auto MakeDefaultScheduler() -> std::unique_ptr<Scheduler<T>>;

} // namespace Mustard::inline Execution

#include "Mustard/Execution/DefaultScheduler.inl"
