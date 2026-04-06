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

#if MUSTARD_SIGNAL_HANDLER

#    include "Mustard/IO/Print.h++"
#    include "Mustard/Utility/FormatToLocalTime.h++"
#    include "Mustard/Utility/FunctionAttribute.h++"
#    include "Mustard/Utility/PrintStackTrace.h++"

#    include "mplr/mplr.hpp"

#    include <chrono>
#    include <csignal>
#    include <cstdio>
#    include <cstdlib>
#    include <ctime>

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
            const auto lineHeader{mplr::available() ?
                                      fmt::format("MPI{}> ", mplr::comm_world().rank()) :
                                      ""};
            const auto ts{fmt::emphasis::bold};
            Print<'E'>(stderr, "\n");
            switch (sig) {
            case SIGINT:
                Print<'E'>(stderr, ts, "{}***** INTERRUPT (SIGINT) received\n", lineHeader);
                break;
            case SIGTERM:
                Print<'E'>(stderr, ts, "{}***** TERMINATE (SIGTERM) received\n", lineHeader);
                break;
            }
            if (mplr::available()) {
                Print<'E'>(stderr, ts, "{}***** in MPI process {} (node: {})\n",
                           lineHeader, mplr::comm_world().rank(), mplr::processor_name());
            }
            Print<'E'>(stderr, ts, "{}***** at {}\n", lineHeader, FormatToLocalTime(now));
            PrintStackTrace(64, 2, stderr, ts);
            Print<'E'>(stderr, "\n");
            switch (sig) {
            case SIGINT:
                Print<'E'>(stderr, ts, "Ctrl-C pressed or an external interrupt received."); // no '\n' looks good for now
                break;
            case SIGTERM:
                Print<'E'>(stderr, ts, "The process is terminated.\n");
                break;
            }
            Print<'E'>(stderr, "\n");
            std::fflush(stderr);
            std::raise(sig);
        }
    } handler{sig};
}

[[noreturn]] auto Mustard_SIGABRT_Handler(int) -> void {
    std::signal(SIGABRT, SIG_DFL);
    const auto now{std::chrono::system_clock::now()};
    const auto lineHeader{mplr::available() ?
                              fmt::format("MPI{}> ", mplr::comm_world().rank()) :
                              ""};
    const auto ts{fmt::emphasis::bold | fg(fmt::color::orange)};
    Print<'E'>(stderr, "\n");
    Print<'E'>(stderr, ts, "{}***** ABORT (SIGABRT) received\n", lineHeader);
    if (mplr::available()) {
        Print<'E'>(stderr, ts, "{}***** in MPI process {} (node: {})\n",
                   lineHeader, mplr::comm_world().rank(), mplr::processor_name());
    }
    Print<'E'>(stderr, ts, "{}***** at {}\n", lineHeader, FormatToLocalTime(now));
    PrintStackTrace(64, 2, stderr, ts);
    Print<'E'>(stderr, "\n");
    Print<'E'>(stderr, ts, "The process is aborted. View the logs just before receiving SIGABRT for more information.\n");
    Print<'E'>(stderr, "\n");
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
            const auto lineHeader{mplr::available() ?
                                      fmt::format("MPI{}> ", mplr::comm_world().rank()) :
                                      ""};
            const auto ts{fmt::emphasis::bold | fg(fmt::color::tomato)};
            Print<'E'>(stderr, "\n");
            switch (sig) {
            case SIGFPE:
                Print<'E'>(stderr, ts, "{}***** ERRONEOUS ARITHMETIC OPERATION (SIGFPE) received\n", lineHeader);
                break;
            case SIGILL:
                Print<'E'>(stderr, ts, "{}***** ILLEGAL INSTRUCTION (SIGILL) received\n", lineHeader);
                break;
            case SIGSEGV:
                Print<'E'>(stderr, ts, "{}***** SEGMENTATION VIOLATION (SIGSEGV) received\n", lineHeader);
                break;
            }
            if (mplr::available()) {
                Print<'E'>(stderr, ts, "{}***** in MPI process {} (node: {})\n",
                           lineHeader, mplr::comm_world().rank(), mplr::processor_name());
            }
            Print<'E'>(stderr, ts, "{}***** at {}\n", lineHeader, FormatToLocalTime(now));
            PrintStackTrace(64, 2, stderr, ts);
            Print<'E'>(stderr, "\n");
            Print<'E'>(stderr, ts, "It is likely that the program has one or more errors. Try seeking help from debugging tools.\n");
            Print<'E'>(stderr, "\n");
            std::fflush(stderr);
            std::raise(sig);
        }
    } handler{sig};
}

} // extern "C"

#endif
