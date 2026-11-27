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

#include "Mustard/Utility/FunctionAttribute.h++"

#include <string>

namespace Mustard::inline Utility {

/// @brief Stack trace text, every line prefixed with the process tag and `"> "`.
///        Without MPI, no prefix is added.
/// @param depth Maximum number of frames to capture.
/// @param skip Number of frames to skip above the immediate caller; 0 starts at the immediate caller.
///        When `context` is non-null, 0 starts at the interrupted instruction instead.
/// @param context Optional signal context (`ucontext_t*`) passed by a `SA_SIGINFO` handler.
/// @param errorAddr Optional fault address (`siginfo_t::si_addr`) passed by a `SA_SIGINFO` handler.
/// @return The stack trace, ending with a newline.
/// @note `errorAddr` is only used by unwinder backends that can unwind from a signal
///       context. Without such a backend, `context` is used to locate the interrupted
///       instruction by its address and the trace starts there; if the address does not
///       appear in the trace, the trace starts above the signal handler.
[[nodiscard]] MUSTARD_NOINLINE auto StackTrace(int depth = 64, int skip = 0, void* context = nullptr, void* errorAddr = nullptr) -> std::string;

} // namespace Mustard::inline Utility
