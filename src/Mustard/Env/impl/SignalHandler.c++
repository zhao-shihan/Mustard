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

#if MUSTARD_SIGNAL_HANDLER

#    include "Mustard/IO/Print.h++"
#    include "Mustard/Utility/FormatToLocalTime.h++"
#    include "Mustard/Utility/FunctionAttribute.h++"
#    include "Mustard/Utility/ProcessTag.h++"
#    include "Mustard/Utility/StackTrace.h++"

#    include "mp1/mp1.hpp"

#    include <chrono>
#    include <csignal>
#    include <cstdio>
#    include <cstdlib>
#    include <ctime>
#    include <string>

namespace Mustard::Env::impl {

extern "C" {

auto Mustard_SIGINT_SIGTERM_Handler(int sig) -> void {
    std::signal(sig, SIG_DFL);
    if (static auto called{false};
        called) {
        std::abort();
    } else {
        called = true;
    }
    static struct Handler {
        MUSTARD_ALWAYS_INLINE Handler(int sig) {
            const auto now{std::chrono::system_clock::now()};
            const auto procTag{mp1::available() ? fmt::format("{}> ", ProcessTag()) : ""};
            std::string buf{"\n"};
            switch (sig) {
            case SIGINT:
                buf += fmt::format("{}***** INTERRUPT (SIGINT received)\n", procTag);
                break;
            case SIGTERM:
                buf += fmt::format("{}***** TERMINATE (SIGTERM received)\n", procTag);
                break;
            }
            buf += fmt::format("{}***** at {}\n", procTag, FormatToLocalTime(now));
            buf += StackTrace(64, 2);
            buf += '\n';
            switch (sig) {
            case SIGINT:
                buf += "Ctrl-C pressed or an external interrupt received."; // no '\n' looks good for now
                break;
            case SIGTERM:
                buf += "The process is terminated.\n";
                break;
            }
            Print<'E'>(stderr, fmt::emphasis::bold, "{}\n", buf);
            std::fflush(stderr);
            std::raise(sig);
        }
    } handler{sig};
}

[[noreturn]] auto Mustard_SIGABRT_Handler(int) -> void {
    std::signal(SIGABRT, SIG_DFL);
    const auto now{std::chrono::system_clock::now()};
    const auto procTag{mp1::available() ? fmt::format("{}> ", ProcessTag()) : ""};
    auto buf{fmt::format("\n"
                         "{}***** ABORT (SIGABRT received)\n"
                         "{}***** at {}\n",
                         procTag, procTag, FormatToLocalTime(now))};
    buf += StackTrace(64, 2);
    buf += "\n"
           "The process is aborted. View the logs just before receiving SIGABRT for more information.\n";
    Print<'E'>(stderr, fmt::emphasis::bold | fg(fmt::color::orange), "{}\n", buf);
    std::fflush(stderr);
    std::abort();
}

auto Mustard_SIGFPE_SIGILL_SIGSEGV_Handler(int sig) -> void {
    std::signal(sig, SIG_DFL);
    if (static auto called{false};
        called) {
        std::abort();
    } else {
        called = true;
    }
    static struct Handler {
        MUSTARD_ALWAYS_INLINE Handler(int sig) {
            const auto now{std::chrono::system_clock::now()};
            const auto procTag{mp1::available() ? fmt::format("{}> ", ProcessTag()) : ""};
            std::string buf{"\n"};
            switch (sig) {
            case SIGFPE:
                buf += fmt::format("{}***** ERRONEOUS ARITHMETIC OPERATION (SIGFPE received)\n", procTag);
                break;
            case SIGILL:
                buf += fmt::format("{}***** ILLEGAL INSTRUCTION (SIGILL received)\n", procTag);
                break;
            case SIGSEGV:
                buf += fmt::format("{}***** SEGMENTATION VIOLATION (SIGSEGV received)\n", procTag);
                break;
            }
            buf += fmt::format("{}***** at {}\n", procTag, FormatToLocalTime(now));
            buf += StackTrace(64, 2);
            buf += "\n"
                   "It is likely that the program has one or more errors. Try seeking help from debugging tools.\n";
            Print<'E'>(stderr, fmt::emphasis::bold | fg(fmt::color::tomato), "{}\n", buf);
            std::fflush(stderr);
            std::raise(sig);
        }
    } handler{sig};
}

} // extern "C"

} // namespace Mustard::Env::impl

#endif
