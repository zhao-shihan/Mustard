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

#include "Mustard/Env/MPIEnv.h++"
#include "Mustard/Env/Print.h++"
#include "Mustard/Utility/PrintStackTrace.h++"

#include "backward.hpp"

#include <climits>
#include <string>
#include <utility>

namespace Mustard::inline Utility {

MUSTARD_NOINLINE auto PrintStackTrace(int depth, int skip, std::ostream& os) -> void {
    const auto trueSkip{skip + 1};
    backward::StackTrace stack;
    stack.load_here(depth + trueSkip);
    backward::TraceResolver resolver;
    resolver.load_stacktrace(stack);

    const auto lineHeader{Env::MPIEnv::Available() ?
                              fmt::format("MPI{}> ", Env::MPIEnv::Instance().CommWorldRank()) :
                              ""};
    auto text{lineHeader + "Stack trace (most recent call last):\n"};
    backward::SnippetFactory snippetFactory;
    for (auto i{std::ssize(stack) - 1}; i >= trueSkip; --i) {
        auto trace{resolver.resolve(stack[i])};
        text += fmt::format("{}#{:<2} {} in {} from {}",
                            lineHeader, i - trueSkip, std::move(trace.addr),
                            trace.object_function.empty() ? "??" : std::move(trace.object_function),
                            trace.object_filename.empty() ? "??" : std::move(trace.object_filename));
        if (auto&& src{trace.source};
            not src.filename.empty()) {
            text += fmt::format(" at {}:{}", src.filename, src.line);
            if (src.col > 0) { text += fmt::format(":{}", src.col); }
            if (auto snippet{snippetFactory.get_snippet(src.filename, src.line, 1)};
                not snippet.empty()) {
                auto&& [line, content] = snippet.front();
                text += fmt::format("\n{} {:7} |{}", lineHeader, std::move(line), std::move(content));
            }
        }
        text += '\n';
    }

    Env::Print(os, "{}", text);
}

} // namespace Mustard::inline Utility
