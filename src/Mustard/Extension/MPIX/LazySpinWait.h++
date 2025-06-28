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

#include "mpl/mpl.hpp"

#include "mpi.h"

#include <span>
#include <vector>

namespace Mustard::inline Extension::MPIX {

auto LazySpinWait(MPI_Request& request, double dutyRatio) -> MPI_Status;
auto LazySpinWait(mpl::irequest& request, double dutyRatio) -> mpl::status_t;
auto LazySpinWait(mpl::irequest&& request, double dutyRatio) -> mpl::status_t;
auto LazySpinWait(mpl::prequest& request, double dutyRatio) -> mpl::status_t;
auto LazySpinWait(mpl::prequest&& request, double dutyRatio) -> mpl::status_t;

struct LazySpinWaitAnyResult {
    mpl::test_result result;
    int index;
};

auto LazySpinWaitAny(std::span<MPI_Request> requests, double dutyRatio) -> LazySpinWaitAnyResult;
auto LazySpinWaitAny(mpl::irequest_pool& requests, double dutyRatio) -> LazySpinWaitAnyResult;
auto LazySpinWaitAny(mpl::prequest_pool& requests, double dutyRatio) -> LazySpinWaitAnyResult;

auto LazySpinWaitAll(std::span<MPI_Request> requests, double dutyRatio) -> void;
auto LazySpinWaitAll(mpl::irequest_pool& requests, double dutyRatio) -> void;
auto LazySpinWaitAll(mpl::prequest_pool& requests, double dutyRatio) -> void;

struct LazySpinWaitSomeResult {
    mpl::test_result result;
    std::vector<int> indices;
};

auto LazySpinWaitSome(std::span<MPI_Request> requests, double dutyRatio) -> LazySpinWaitSomeResult;
auto LazySpinWaitSome(mpl::irequest_pool& requests, double dutyRatio) -> LazySpinWaitSomeResult;
auto LazySpinWaitSome(mpl::prequest_pool& requests, double dutyRatio) -> LazySpinWaitSomeResult;

} // namespace Mustard::inline Extension::MPIX
