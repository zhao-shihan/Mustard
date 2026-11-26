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

#include "Mustard/Utility/ProcessTag.h++"
#include "Mustard/Utility/StackTrace.h++"

#include "backward.hpp"

#include "mp1/mp1.hpp"

#include "fmt/format.h"

#include <algorithm>
#include <climits>
#include <string>
#include <utility>

#if not defined BACKWARD_WALKER_LIBUNWIND and not defined _WIN32
#    include <ucontext.h>
#endif

namespace Mustard::inline Utility {

namespace {

#if not defined BACKWARD_WALKER_LIBUNWIND

auto InterruptedPC(void* context) -> void* {
    // Without libunwind, the unwinder starts in this file and the number of frames above
    // the interrupted instruction depends on how the handler and backward-cpp happen to
    // be inlined, so locate the interrupted instruction by its address from the signal
    // context instead of counting frames.
#    if defined __linux__ and defined __x86_64__ and defined REG_RIP
    return reinterpret_cast<void*>(static_cast<ucontext_t*>(context)->uc_mcontext.gregs[REG_RIP]);
#    elif defined __linux__ and defined __aarch64__
    return reinterpret_cast<void*>(static_cast<ucontext_t*>(context)->uc_mcontext.pc);
#    else
    static_cast<void>(context);
    return nullptr;
#    endif
}

#endif

} // namespace

[[nodiscard]] MUSTARD_NOINLINE auto StackTrace(int depth, int skip, void* context, void* errorAddr) -> std::string {
    backward::StackTrace stack;
    if (context == nullptr) {
        skip += 1; // Skip StackTrace itself.
        stack.load_here(depth + skip, context, errorAddr);
    } else {
#if not defined BACKWARD_WALKER_LIBUNWIND
        // Without libunwind the unwinder cannot use the signal context. Locate the
        // interrupted instruction by its address instead, which does not depend on how
        // the signal handler happens to be inlined.
        const auto* interruptedPC{InterruptedPC(context)};
        stack.load_here(depth + 8, context, errorAddr);
        if (interruptedPC != nullptr) {
            for (int i{}; i < std::ssize(stack); ++i) {
                if (stack[i].addr == interruptedPC) {
                    skip = i;
                    break;
                }
            }
        }
        if (skip == 0) {
            skip += 1; // The interrupted instruction is unknown; skip StackTrace itself.
        }
#else
        stack.load_here(depth + skip, context, errorAddr);
#endif
    }
    backward::TraceResolver resolver;
    resolver.load_stacktrace(stack);

    const auto procTag{mp1::available() ? fmt::format("{}> ", ProcessTag()) : ""};
    auto text{procTag + "Stack trace (most recent call last):\n"};
    backward::SnippetFactory snippetFactory;
    for (auto i{std::min<std::ptrdiff_t>(std::ssize(stack) - 1, skip + depth - 1)}; i >= skip; --i) {
        auto trace{resolver.resolve(stack[i])};
        text += fmt::format("{}#{:<2} {} in {} from {}",
                            procTag, i - skip, std::move(trace.addr),
                            trace.object_function.empty() ? "??" : std::move(trace.object_function),
                            trace.object_filename.empty() ? "??" : std::move(trace.object_filename));
        if (auto&& src{trace.source};
            not src.filename.empty()) {
            text += fmt::format(" at {}:{}", src.filename, src.line);
            if (src.col > 0) {
                text += fmt::format(":{}", src.col);
            }
            if (auto snippet{snippetFactory.get_snippet(src.filename, src.line, 1)};
                not snippet.empty()) {
                auto&& [line, content] = snippet.front();
                text += fmt::format("\n{} {:7} |{}", procTag, std::move(line), std::move(content));
            }
        }
        text += '\n';
    }
    return text;
}

} // namespace Mustard::inline Utility
