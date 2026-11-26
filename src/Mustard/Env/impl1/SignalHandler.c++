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

#include "Mustard/Utility/FormatToLocalTime.h++"
#include "Mustard/Utility/ProcessTag.h++"
#include "Mustard/Utility/StackTrace.h++"

#include "mp1/mp1.hpp"

#include "fmt/color.h"
#include "fmt/format.h"

#include <atomic>
#include <cerrno>
#include <chrono>
#include <csignal>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <string_view>

#ifndef _WIN32
#    include <unistd.h>
#endif

namespace Mustard::Env::impl1 {

namespace {
namespace SignalHandler {

// Every signal taken over in this file terminates the process. Handling is best-effort:
// formatting and symbolization are not async-signal-safe. A signal that should instead
// resume the interrupted instruction needs its own Behavior, its own entry point (which
// must not be [[noreturn]], and must save and restore errno) and its own row below.
std::atomic_flag terminatingSignalHandled{};

auto WriteStderrUnlocked(std::string_view text) -> void {
    // Async-signal-safe on POSIX: no stdio lock is taken, so a thread that died while
    // holding stderr cannot deadlock the report. Windows has no equivalent, so stdio is
    // used there as a best effort.
#ifdef _WIN32
    std::fwrite(text.data(), 1, text.size(), stderr);
    std::fflush(stderr);
#else
    while (not text.empty()) {
        // write(2) may write less than requested, in particular onto a pipe.
        const auto written{::write(STDERR_FILENO, text.data(), text.size())};
        if (written > 0) {
            text.remove_prefix(static_cast<std::size_t>(written));
        } else if (written < 0 and errno == EINTR) {
            continue;
        } else {
            return;
        }
    }
#endif
}

enum struct Behavior {
    Terminating, ///< Produce a report, then die with the signal's default action.
};

enum struct AddressType {
    None,        ///< si_addr carries no useful address.
    Accessed,    ///< si_addr is the accessed memory address (SIGSEGV, SIGBUS).
    Instruction, ///< si_addr is the faulting instruction address (SIGFPE, SIGILL).
};

constexpr auto AddressLabel(AddressType addressType) -> std::string_view {
    switch (addressType) {
    case AddressType::Accessed:
        return "fault address";
    case AddressType::Instruction:
        return "faulting instruction address";
    default:
        return {};
    }
}

struct Policy {
    int signal;
    Behavior behavior;
    AddressType addressType;
    fmt::text_style style;
    std::string_view head;
    std::string_view tail;
    bool keepInheritedDisposition;
};

// One row per signal taken over; every signal absent from this table keeps its default
// or inherited disposition on purpose:
//  * SIGTSTP/SIGTTIN/SIGTTOU -- catching a stop signal keeps the process from being
//    stopped, which breaks Ctrl-Z and background terminal I/O;
//  * SIGCONT -- the kernel continues a stopped process even when SIGCONT is ignored or
//    blocked, so a handler could only add noise;
//  * SIGCHLD/SIGURG/SIGWINCH -- ignored by default, and catching them would report every
//    child exit, urgent socket data and terminal resize (POSIX also leaves the behavior
//    unspecified when SIGCHLD is set to SIG_IGN);
//  * SIGUSR1/SIGUSR2 -- user-defined and reserved for the application;
//  * SIGALRM/SIGVTALRM -- owned by alarm()/setitimer() and by profilers;
//  * SIGTRAP -- owned by debuggers and perf;
//  * SIGSYS -- seccomp SECCOMP_RET_TRAP handlers emulate system calls there;
//  * SIGPIPE -- ignoring it (EPIPE instead of death) is the caller's decision, and
//    reporting onto a broken stderr would raise SIGPIPE again.

// Hand-formatted: clang-format's AlignArrayOfStructures realigns every row whenever one
// row changes.
// clang-format off
constexpr Policy gPolicy[]{
    {.signal = SIGINT,
     .behavior = Behavior::Terminating,
     .addressType = AddressType::None,
     .style = fmt::emphasis::bold,
     .head = "*** INTERRUPT (SIGINT) ***",
     .tail = "*** The process is interrupted ***", // Usually triggered by Ctrl+C, so no extra newline.
     .keepInheritedDisposition = false},
    {.signal = SIGTERM,
     .behavior = Behavior::Terminating,
     .addressType = AddressType::None,
     .style = fmt::emphasis::bold,
     .head = "*** TERMINATE (SIGTERM) ***",
     .tail = "*** The process is terminated ***\n",
     .keepInheritedDisposition = false},
#ifdef SIGHUP
    {.signal = SIGHUP,
     .behavior = Behavior::Terminating,
     .addressType = AddressType::None,
     .style = fmt::emphasis::bold,
     .head = "*** HANGUP (SIGHUP) ***",
     .tail = "*** The process is hung up ***\n",
     .keepInheritedDisposition = true},
#endif
#ifdef SIGQUIT
    {.signal = SIGQUIT,
     .behavior = Behavior::Terminating,
     .addressType = AddressType::None,
     .style = fmt::emphasis::bold,
     .head = "*** QUIT (SIGQUIT) ***",
     .tail = "*** The process is quit ***\n",
     .keepInheritedDisposition = true},
#endif
    {.signal = SIGABRT,
     .behavior = Behavior::Terminating,
     .addressType = AddressType::None,
     .style = fmt::emphasis::bold | fg(fmt::color::orange),
     .head = "*** ABORT (SIGABRT) ***",
     .tail = "*** The process is aborted ***\n",
     .keepInheritedDisposition = false},
#ifdef SIGBUS
    {.signal = SIGBUS,
     .behavior = Behavior::Terminating,
     .addressType = AddressType::Accessed,
     .style = fmt::emphasis::bold | fg(fmt::color::tomato),
     .head = "*** BUS ERROR (SIGBUS) ***",
     .tail = "*** The process encountered a bus error ***\n",
     .keepInheritedDisposition = false},
#endif
    {.signal = SIGFPE,
     .behavior = Behavior::Terminating,
     .addressType = AddressType::Instruction,
     .style = fmt::emphasis::bold | fg(fmt::color::tomato),
     .head = "*** ERRONEOUS ARITHMETIC OPERATION (SIGFPE) ***",
     .tail = "*** The process encountered an erroneous arithmetic operation ***\n",
     .keepInheritedDisposition = false},
    {.signal = SIGILL,
     .behavior = Behavior::Terminating,
     .addressType = AddressType::Instruction,
     .style = fmt::emphasis::bold | fg(fmt::color::tomato),
     .head = "*** ILLEGAL INSTRUCTION (SIGILL) ***",
     .tail = "*** The process encountered an illegal instruction ***\n",
     .keepInheritedDisposition = false},
    {.signal = SIGSEGV,
     .behavior = Behavior::Terminating,
     .addressType = AddressType::Accessed,
     .style = fmt::emphasis::bold | fg(fmt::color::tomato),
     .head = "*** SEGMENTATION VIOLATION (SIGSEGV) ***",
     .tail = "*** The process encountered a segmentation violation ***\n",
     .keepInheritedDisposition = false},
#ifdef SIGXCPU
    {.signal = SIGXCPU,
     .behavior = Behavior::Terminating,
     .addressType = AddressType::None,
     .style = fmt::emphasis::bold | fg(fmt::color::orange),
     .head = "*** CPU TIME LIMIT EXCEEDED (SIGXCPU) ***",
     .tail = "*** The process exceeded its CPU time limit ***\n",
     .keepInheritedDisposition = false},
#endif
#ifdef SIGXFSZ
    {.signal = SIGXFSZ,
     .behavior = Behavior::Terminating,
     .addressType = AddressType::None,
     .style = fmt::emphasis::bold | fg(fmt::color::orange),
     .head = "*** FILE SIZE LIMIT EXCEEDED (SIGXFSZ) ***",
     .tail = "*** The process exceeded its file size limit ***\n",
     .keepInheritedDisposition = false},
#endif
};
// clang-format on

constexpr auto FindPolicy(int sig) -> const Policy* {
    for (const auto& policy : gPolicy) {
        if (policy.signal == sig) {
            return &policy;
        }
    }
    return nullptr;
}

auto UnblockSignal([[maybe_unused]] int sig) -> void {
#ifndef _WIN32
    sigset_t set{};
    sigemptyset(&set);
    sigaddset(&set, sig);
    // pthread_sigmask rather than sigprocmask: the behavior of sigprocmask is unspecified
    // in a multi-threaded process. raise() is thread-directed, so it is the calling
    // thread's mask that has to let the signal through.
    pthread_sigmask(SIG_UNBLOCK, &set, nullptr);
#endif
}

auto ResetToDefault([[maybe_unused]] int sig) -> void {
#ifdef _WIN32
    std::signal(sig, SIG_DFL);
#else
    // sigaction() is used to install the handler and POSIX advises against mixing it with
    // signal() on the same signal. This reset is also what makes SIGILL terminate:
    // SA_RESETHAND cannot reset SIGILL (or SIGTRAP) on entry to the handler.
    struct sigaction action{};
    action.sa_handler = SIG_DFL;
    sigemptyset(&action.sa_mask);
    sigaction(sig, &action, nullptr);
#endif
}

[[noreturn]] auto HandleTerminatingSignal(int sig, void* context, void* errorAddr, int errorCode) -> void {
    constexpr auto stackTraceDepth{64};
#ifdef _WIN32
    // Windows does not provide a signal context; skip the signal-handler frames.
    constexpr auto stackTraceSkip{2};
#else
    constexpr auto stackTraceSkip{0};
#endif

    ResetToDefault(sig);
    if (terminatingSignalHandled.test_and_set()) {
        // Another signal arrived while the report was being produced. Nothing on the
        // report path is async-signal-safe, so leave a one-line note and die at once
        // instead of nesting a second report.
        WriteStderrUnlocked("\n*** Another signal received while reporting, terminating immediately ***\n");
        UnblockSignal(sig);
        std::raise(sig);
        std::_Exit(EXIT_FAILURE); // raise returns only if the signal is still blocked or ignored
    }

    const auto policy{FindPolicy(sig)};
    if (policy == nullptr) {
        std::_Exit(EXIT_FAILURE);
    }

    const auto now{std::chrono::system_clock::now()};
    const auto procTag{mp1::available() ? fmt::format("{}> ", ProcessTag()) : ""};
    // si_addr is only meaningful for kernel-generated faults; a signal sent by
    // kill/raise/pthread_kill (si_code <= 0) carries no address.
    const auto addressLabel{AddressLabel(policy->addressType)};
    const auto addressLine{
        addressLabel.empty() or errorCode <= 0 ?
            std::string{} :
            fmt::format("{}*** {} 0x{:x}\n", procTag, addressLabel,
                        reinterpret_cast<std::uintptr_t>(errorAddr))};
    const auto report{fmt::format(
        policy->style,
        "\n"
        "{}{}\n"
        "{}*** at {}\n"
        "{}"
        "{}"
        "{}{}\n",
        procTag, policy->head,
        procTag, FormatToLocalTime(now),
        addressLine,
        // backward-cpp's libunwind backend treats errorAddr as the address of a bad
        // function pointer and replaces the interrupted IP with the return address on the
        // stack. Only memory-access faults know such an address; for SIGILL/SIGFPE
        // si_addr is the faulting instruction itself, so it must not reach the unwinder.
        StackTrace(stackTraceDepth, stackTraceSkip, context,
                   policy->addressType == AddressType::Accessed ? errorAddr : nullptr),
        procTag, policy->tail)};
    WriteStderrUnlocked(report);

    if (sig == SIGABRT) {
        // abort() overrides blocking and ignoring SIGABRT and never returns. The
        // disposition is already SIG_DFL, so this terminates with the SIGABRT status.
        std::abort();
    }
    // Deliver the signal again with the kernel default action. Unblock it first: the
    // handled signal is blocked inside its own handler, so a plain raise() would only
    // be delivered after this handler returns, and the caller would exit before that.
    UnblockSignal(sig);
    std::raise(sig);
    std::_Exit(EXIT_FAILURE); // raise returns only if the signal cannot terminate
}

#ifndef _WIN32

auto KeepInheritedDisposition(const Policy& policy) -> bool {
    // SIGHUP and SIGQUIT are the two signals a parent deliberately ignores to keep a
    // process alive across a terminal hangup or as a background job of a non-interactive
    // shell (nohup, asynchronous lists), so an inherited SIG_IGN is respected there. Every
    // other signal is installed unconditionally: an explicit kill should still report.
    // The hardware faults in particular must never be skipped, since ignoring a
    // kernel-generated SIGSEGV, SIGBUS, SIGFPE or SIGILL is undefined behavior.
    if (not policy.keepInheritedDisposition) {
        return false;
    }
    struct sigaction current{};
    return sigaction(policy.signal, nullptr, &current) == 0 and current.sa_handler == SIG_IGN;
}

auto EnsureAlternateSignalStack() -> void {
    // Large enough for formatting and symbolization inside a signal handler; matches
    // backward-cpp's alternate signal stack. Pages are touched lazily.
    constexpr auto signalStackSize{8 * 1024 * 1024};
    static auto signalStack{static_cast<char*>(std::malloc(signalStackSize))};
    if (signalStack == nullptr) {
        return;
    }
    // sigaltstack is per-thread; this covers the thread constructing the environment.
    stack_t alternateStack{};
    alternateStack.ss_sp = signalStack;
    alternateStack.ss_size = signalStackSize;
    alternateStack.ss_flags = 0;
    sigaltstack(&alternateStack, nullptr);
}

auto InstallPOSIXSignalHandler(const Policy& policy, auto (*handler)(int, siginfo_t*, void*)->void) -> void {
    if (KeepInheritedDisposition(policy)) {
        return;
    }
    struct sigaction action{};
    action.sa_sigaction = handler;
    action.sa_flags = SA_SIGINFO;
#    ifdef SA_ONSTACK
    action.sa_flags |= SA_ONSTACK;
#    endif
#    ifdef SA_RESETHAND
    action.sa_flags |= SA_RESETHAND;
#    endif
    sigemptyset(&action.sa_mask);
    // Block synchronous faults while reporting, so that a fault inside the reporter
    // cannot recurse. POSIX leaves the result undefined if such a signal is generated
    // while blocked, so this relies on the Linux behavior of killing the process.
    // Termination requests (SIGINT, SIGTERM, ...) stay deliverable, so that a stuck
    // report can still be aborted.
    sigaddset(&action.sa_mask, SIGABRT);
    sigaddset(&action.sa_mask, SIGFPE);
    sigaddset(&action.sa_mask, SIGILL);
    sigaddset(&action.sa_mask, SIGSEGV);
#    ifdef SIGBUS
    sigaddset(&action.sa_mask, SIGBUS);
#    endif
    if (sigaction(policy.signal, &action, nullptr) != 0) {
        fmt::print(stderr, fg(fmt::color::orange),
                   "Warning: Failed to install the signal handler for signal {}.\n", policy.signal);
    }
}

#endif

} // namespace SignalHandler
} // namespace

extern "C" {

// Entry point for the terminating signals; see HandleTerminatingSignal for the contract.

#ifdef _WIN32

[[noreturn]] auto MustardTerminatingSignalHandler(int sig) -> void {
    SignalHandler::HandleTerminatingSignal(sig, nullptr, nullptr, 0);
}

#else

[[noreturn]] auto MustardTerminatingSignalHandler(int sig, siginfo_t* info, void* context) -> void {
    SignalHandler::HandleTerminatingSignal(
        sig, context,
        info != nullptr ? info->si_addr : nullptr,
        info != nullptr ? info->si_code : 0);
}

#endif

} // extern "C"

auto InstallSignalHandlers() -> void {
#ifndef _WIN32
    SignalHandler::EnsureAlternateSignalStack();
#endif
    for (const auto& policy : SignalHandler::gPolicy) {
        switch (policy.behavior) {
        case SignalHandler::Behavior::Terminating:
#ifdef _WIN32
            std::signal(policy.signal, MustardTerminatingSignalHandler);
#else
            SignalHandler::InstallPOSIXSignalHandler(policy, MustardTerminatingSignalHandler);
#endif
            break;
        }
    }
}

} // namespace Mustard::Env::impl1
