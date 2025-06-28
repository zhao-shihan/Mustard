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

#include "Mustard/Extension/MPIX/LazySpinWait.h++"
#include "Mustard/Utility/InlineMacro.h++"
#include "Mustard/Utility/PrettyLog.h++"

#include "muc/chrono"

#include "fmt/core.h"

#include <chrono>
#include <stdexcept>
#include <thread>
#include <utility>

namespace Mustard::inline Extension::MPIX {

namespace internal {
namespace {

MUSTARD_ALWAYS_INLINE auto CheckCalculateSleepRatio(double dutyRatio) -> double {
    if (dutyRatio <= 0 or dutyRatio > 1) {
        Mustard::Throw<std::invalid_argument>(fmt::format("Duty ratio {} out of (0,1]", dutyRatio));
    }
    return 1 / dutyRatio - 1;
}

} // namespace
} // namespace internal

auto LazySpinWait(MPI_Request& request, double dutyRatio) -> MPI_Status {
    const auto sleepRatio{internal::CheckCalculateSleepRatio(dutyRatio)};
    int completed{};
    MPI_Status status{};
    while (true) {
        const auto t0{muc::chrono::steady_high_resolution_clock::now()};
        MPI_Test(&request, &completed, &status);
        if (completed) {
            return status;
        }
        const auto t1{muc::chrono::steady_high_resolution_clock::now()};
        std::this_thread::sleep_for(sleepRatio * (t1 - t0));
    }
}

namespace internal {
namespace {

template<typename R>
MUSTARD_ALWAYS_INLINE auto LazySpinWait(R& request, double dutyRatio) -> mpl::status_t {
    const auto sleepRatio{internal::CheckCalculateSleepRatio(dutyRatio)};
    while (true) {
        const auto t0{muc::chrono::steady_high_resolution_clock::now()};
        const auto status{request.test()};
        if (status) {
            return *status;
        }
        const auto t1{muc::chrono::steady_high_resolution_clock::now()};
        std::this_thread::sleep_for(sleepRatio * (t1 - t0));
    }
}

} // namespace
} // namespace internal

auto LazySpinWait(mpl::irequest& request, double dutyRatio) -> mpl::status_t {
    return internal::LazySpinWait(request, dutyRatio);
}

auto LazySpinWait(mpl::irequest&& request, double dutyRatio) -> mpl::status_t {
    return internal::LazySpinWait(request, dutyRatio);
}

auto LazySpinWait(mpl::prequest& request, double dutyRatio) -> mpl::status_t {
    return internal::LazySpinWait(request, dutyRatio);
}

auto LazySpinWait(mpl::prequest&& request, double dutyRatio) -> mpl::status_t {
    return internal::LazySpinWait(request, dutyRatio);
}

auto LazySpinWaitAny(std::span<MPI_Request> requests, double dutyRatio) -> LazySpinWaitAnyResult {
    const auto sleepRatio{internal::CheckCalculateSleepRatio(dutyRatio)};
    int index{};
    int completed{};
    while (true) {
        const auto t0{muc::chrono::steady_high_resolution_clock::now()};
        MPI_Testany(requests.size(), requests.data(), &index, &completed, MPI_STATUS_IGNORE);
        if (completed) {
            if (index == MPI_UNDEFINED) {
                return {.result{mpl::test_result::no_active_requests},
                        .index{static_cast<int>(requests.size())}};
            } else {
                return {.result{mpl::test_result::completed},
                        .index{index}};
            }
        }
        const auto t1{muc::chrono::steady_high_resolution_clock::now()};
        std::this_thread::sleep_for(sleepRatio * (t1 - t0));
    }
}

namespace internal {
namespace {

template<typename P>
MUSTARD_ALWAYS_INLINE auto LazySpinWaitAny(P& requests, double dutyRatio) -> LazySpinWaitAnyResult {
    const auto sleepRatio{internal::CheckCalculateSleepRatio(dutyRatio)};
    while (true) {
        const auto t0{muc::chrono::steady_high_resolution_clock::now()};
        const auto [result, index]{requests.testany()};
        if (result != mpl::test_result::no_completed) {
            return {.result{result},
                    .index{static_cast<int>(index)}};
        }
        const auto t1{muc::chrono::steady_high_resolution_clock::now()};
        std::this_thread::sleep_for(sleepRatio * (t1 - t0));
    }
}

} // namespace
} // namespace internal

auto LazySpinWaitAny(mpl::irequest_pool& requests, double dutyRatio) -> LazySpinWaitAnyResult {
    return internal::LazySpinWaitAny(requests, dutyRatio);
}

auto LazySpinWaitAny(mpl::prequest_pool& requests, double dutyRatio) -> LazySpinWaitAnyResult {
    return internal::LazySpinWaitAny(requests, dutyRatio);
}

auto LazySpinWaitAll(std::span<MPI_Request> requests, double dutyRatio) -> void {
    const auto sleepRatio{internal::CheckCalculateSleepRatio(dutyRatio)};
    int completed{};
    while (true) {
        const auto t0{muc::chrono::steady_high_resolution_clock::now()};
        MPI_Testall(requests.size(), requests.data(), &completed, MPI_STATUSES_IGNORE);
        if (completed) {
            return;
        }
        const auto t1{muc::chrono::steady_high_resolution_clock::now()};
        std::this_thread::sleep_for(sleepRatio * (t1 - t0));
    }
}

namespace internal {
namespace {

template<typename P>
MUSTARD_ALWAYS_INLINE auto LazySpinWaitAll(P& requests, double dutyRatio) -> void {
    const auto sleepRatio{internal::CheckCalculateSleepRatio(dutyRatio)};
    while (true) {
        const auto t0{muc::chrono::steady_high_resolution_clock::now()};
        if (requests.testall()) {
            return;
        }
        const auto t1{muc::chrono::steady_high_resolution_clock::now()};
        std::this_thread::sleep_for(sleepRatio * (t1 - t0));
    }
}

} // namespace
} // namespace internal

auto LazySpinWaitAll(mpl::irequest_pool& requests, double dutyRatio) -> void {
    internal::LazySpinWaitAll(requests, dutyRatio);
}

auto LazySpinWaitAll(mpl::prequest_pool& requests, double dutyRatio) -> void {
    internal::LazySpinWaitAll(requests, dutyRatio);
}

auto LazySpinWaitSome(std::span<MPI_Request> requests, double dutyRatio) -> LazySpinWaitSomeResult {
    const auto sleepRatio{internal::CheckCalculateSleepRatio(dutyRatio)};
    int count{};
    std::vector<int> indices(requests.size());
    while (true) {
        const auto t0{muc::chrono::steady_high_resolution_clock::now()};
        MPI_Testsome(requests.size(), requests.data(), &count, indices.data(), MPI_STATUSES_IGNORE);
        if (count == MPI_UNDEFINED) {
            return {.result{mpl::test_result::no_active_requests},
                    .indices{}};
        }
        if (count != 0) {
            indices.resize(count); // shrink
            return {.result{mpl::test_result::completed},
                    .indices{std::move(indices)}};
        }
        const auto t1{muc::chrono::steady_high_resolution_clock::now()};
        std::this_thread::sleep_for(sleepRatio * (t1 - t0));
    }
}

namespace internal {
namespace {

template<typename P>
MUSTARD_ALWAYS_INLINE auto LazySpinWaitSome(P& requests, double dutyRatio) -> LazySpinWaitSomeResult {
    const auto sleepRatio{internal::CheckCalculateSleepRatio(dutyRatio)};
    while (true) {
        const auto t0{muc::chrono::steady_high_resolution_clock::now()};
        auto [result, indices]{requests.testsome()};
        if (result != mpl::test_result::no_completed) {
            std::vector<int> ret(indices.begin(), indices.end());
            return {.result{result},
                    .indices{std::move(ret)}};
        }
        const auto t1{muc::chrono::steady_high_resolution_clock::now()};
        std::this_thread::sleep_for(sleepRatio * (t1 - t0));
    }
}

} // namespace
} // namespace internal

auto LazySpinWaitSome(mpl::irequest_pool& requests, double dutyRatio) -> LazySpinWaitSomeResult {
    return internal::LazySpinWaitSome(requests, dutyRatio);
}

auto LazySpinWaitSome(mpl::prequest_pool& requests, double dutyRatio) -> LazySpinWaitSomeResult {
    return internal::LazySpinWaitSome(requests, dutyRatio);
}

} // namespace Mustard::inline Extension::MPIX
