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

#include <chrono>
#include <concepts>
#include <thread>
#include <utility>

namespace Mustard::inline Extension::MPIX {

template<typename R, typename P>
auto LazySpinWait(MPI_Request& request, std::chrono::duration<R, P> pollInterval) -> MPI_Status;

template<typename T, typename R, typename P>
    requires std::derived_from<T, mpl::irequest> or std::derived_from<T, mpl::prequest>
auto LazySpinWait(T& request, std::chrono::duration<R, P> pollInterval) -> mpl::status_t;
template<typename T, typename R, typename P>
    requires std::derived_from<T, mpl::irequest> or std::derived_from<T, mpl::prequest>
auto LazySpinWait(T&& request, std::chrono::duration<R, P> pollInterval) -> mpl::status_t;

} // namespace Mustard::inline Extension::MPIX

#include "Mustard/Extension/MPIX/LazySpinWait.inl"
