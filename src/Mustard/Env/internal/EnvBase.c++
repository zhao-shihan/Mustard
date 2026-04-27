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

#include "Mustard/Env/ObjectRegistry/internal/SingletonPool.h++"
#include "Mustard/Env/ObjectRegistry/internal/WeakSingletonPool.h++"
#include "Mustard/Env/internal/EnvBase.h++"
#include "Mustard/IO/PrettyLog.h++"

#include "muc/bit"
#include "muc/utility"

#include "fmt/color.h"

#include <exception>
#include <limits>
#include <stdexcept>

#ifdef MUSTARD_SIGNAL_HANDLER

#    include <csignal>

extern "C" {
auto Mustard_SIGINT_SIGTERM_Handler(int sig) -> void;
[[noreturn]] auto Mustard_SIGABRT_Handler(int) -> void;
auto Mustard_SIGFPE_SIGILL_SIGSEGV_Handler(int sig) -> void;
} // extern "C"

#endif

namespace Mustard::Env::internal {

[[noreturn]] auto TerminateHandler() -> void;

EnvBase::EnvBase() :
    NonCopyableBase{},
    fWeakSingletonPool{},
    fSingletonPool{} {

    static_assert("三清在上 运行稳定 结果无偏");

    CheckFundamentalType();

    std::set_terminate(TerminateHandler);

#if MUSTARD_SIGNAL_HANDLER
    std::signal(SIGABRT, Mustard_SIGABRT_Handler);
    std::signal(SIGFPE, Mustard_SIGFPE_SIGILL_SIGSEGV_Handler);
    std::signal(SIGILL, Mustard_SIGFPE_SIGILL_SIGSEGV_Handler);
    std::signal(SIGINT, Mustard_SIGINT_SIGTERM_Handler);
    std::signal(SIGSEGV, Mustard_SIGFPE_SIGILL_SIGSEGV_Handler);
    std::signal(SIGTERM, Mustard_SIGINT_SIGTERM_Handler);
#endif

    if (static bool gInstantiated{false};
        gInstantiated) {
        Throw<std::runtime_error>("Trying to construct environment twice");
    } else {
        gInstantiated = true;
    }

    fWeakSingletonPool = std::make_unique<ObjectRegistry::internal::WeakSingletonPool>();
    fSingletonPool = std::make_unique<ObjectRegistry::internal::SingletonPool>();
}

EnvBase::~EnvBase() = default;

auto EnvBase::CheckFundamentalType() -> void {
    constexpr auto lp32{muc::bit_size<char> == 8 and
                        muc::bit_size<short> == 16 and
                        muc::bit_size<int> == 16 and
                        muc::bit_size<long> == 32 and
                        muc::bit_size<long long> == 64 and
                        muc::bit_size<void*> == 32};
    constexpr auto ilp32{muc::bit_size<char> == 8 and
                         muc::bit_size<short> == 16 and
                         muc::bit_size<int> == 32 and
                         muc::bit_size<long> == 32 and
                         muc::bit_size<long long> == 64 and
                         muc::bit_size<void*> == 32};
    constexpr auto llp64{muc::bit_size<char> == 8 and
                         muc::bit_size<short> == 16 and
                         muc::bit_size<int> == 32 and
                         muc::bit_size<long> == 32 and
                         muc::bit_size<long long> == 64 and
                         muc::bit_size<void*> == 64};
    constexpr auto lp64{muc::bit_size<char> == 8 and
                        muc::bit_size<short> == 16 and
                        muc::bit_size<int> == 32 and
                        muc::bit_size<long> == 64 and
                        muc::bit_size<long long> == 64 and
                        muc::bit_size<void*> == 64};
    if constexpr (not lp64) {
        if constexpr (llp64) {
            fmt::print(stderr, fg(fmt::color::orange), "Warning: The fundamental data model is LLP64 (not LP64)\n");
        } else if constexpr (ilp32) {
            fmt::print(stderr, fg(fmt::color::orange), "Warning: The fundamental data model is ILP32 (not LP64)\n");
        } else if constexpr (lp32) {
            fmt::print(stderr, fg(fmt::color::orange), "Warning: The fundamental data model is LP32 (not LP64)\n");
        } else {
            fmt::print(stderr, fg(fmt::color::orange), "Warning: Using a rare fundamental data model "
                                                       "[{}-bits char, {}-bits short, {}-bits int, {}-bits long, {}-bits long long, {}-bits pointer], "
                                                       "run at your own risk\n",
                       muc::bit_size<char>, muc::bit_size<short>, muc::bit_size<int>, muc::bit_size<long>, muc::bit_size<long long>, muc::bit_size<void*>);
        }
    }
    if constexpr (not std::numeric_limits<float>::is_iec559) {
        fmt::print(stderr, fg(fmt::color::orange), "Warning: 'float' does not fulfill the requirements of IEC 559 (IEEE 754)\n");
    }
    if constexpr (not std::numeric_limits<double>::is_iec559) {
        fmt::print(stderr, fg(fmt::color::orange), "Warning: 'double' does not fulfill the requirements of IEC 559 (IEEE 754)\n");
    }
    if constexpr (not std::numeric_limits<long double>::is_iec559) {
        fmt::print(stderr, fg(fmt::color::orange), "Warning: 'long double' does not fulfill the requirements of IEC 559 (IEEE 754)\n");
    }
}

} // namespace Mustard::Env::internal
