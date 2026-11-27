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

#include "Mustard/CLI/BasicCLI.h++"
#include "Mustard/Env/MPIEnv.h++"
#include "Mustard/Testing/TestSignalHandler/TestSignalHandler.h++"

#include "fmt/format.h"

#include <algorithm>
#include <array>
#include <cerrno>
#include <chrono>
#include <climits>
#include <csignal>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <string_view>
#include <system_error>
#include <thread>
#include <vector>

#if defined __APPLE__
#    include <mach-o/dyld.h>
#endif
#if defined __unix__ or defined __APPLE__
#    include <sys/resource.h>
#    include <sys/wait.h>
#    include <unistd.h>
#endif

namespace Mustard::Testing {

namespace {

#if defined __unix__ or defined __APPLE__

using namespace std::chrono_literals;

constexpr auto childTimeout{120s};
constexpr auto maxChildOutput{std::size_t{1} << 20};

/// @brief Env var read by the child process to know what it should do.
constexpr auto childTaskEnvVar{"MUSTARD_TEST_SIGNAL"};

auto ParseSignal(std::string_view name) -> int {
    if (name == "INT") {
        return SIGINT;
    }
    if (name == "TERM") {
        return SIGTERM;
    }
    if (name == "ABRT") {
        return SIGABRT;
    }
    if (name == "FPE") {
        return SIGFPE;
    }
    if (name == "ILL") {
        return SIGILL;
    }
    if (name == "SEGV") {
        return SIGSEGV;
    }
#    ifdef SIGHUP
    if (name == "HUP") {
        return SIGHUP;
    }
#    endif
#    ifdef SIGQUIT
    if (name == "QUIT") {
        return SIGQUIT;
    }
#    endif
#    ifdef SIGBUS
    if (name == "BUS") {
        return SIGBUS;
    }
#    endif
#    ifdef SIGXCPU
    if (name == "XCPU") {
        return SIGXCPU;
    }
#    endif
#    ifdef SIGXFSZ
    if (name == "XFSZ") {
        return SIGXFSZ;
    }
#    endif
    return 0;
}

auto RunChild(int argc, char* argv[], std::string_view task) -> int {
    // The inherited dispositions (a background job inherits SIGINT and SIGQUIT ignored
    // from the shell, wrappers may ignore other signals) must not decide what a case
    // exercises, so each case sets its own before the environment installs the handlers.
    if (task == "IGNORE_HUP") {
#    ifdef SIGHUP
        std::signal(SIGHUP, SIG_IGN);
#    endif
    } else if (const auto sig{ParseSignal(task)}; sig != 0) {
        std::signal(sig, SIG_DFL);
    }

    Mustard::CLI::BasicCLI<> cli;
    Mustard::Env::MPIEnv env{argc, argv, cli};

    if (task == "FAULT_NULL") {
        // The address comes from the environment so that the compiler cannot warn about
        // (or fold away) the deliberate fault.
        const auto addressText{std::getenv("MUSTARD_TEST_FAULT_ADDRESS")};
        const auto address{static_cast<std::uintptr_t>(
            addressText != nullptr ? std::strtoull(addressText, nullptr, 0) : 0)};
        *reinterpret_cast<volatile int*>(address) = 1;
    } else if (task == "FAULT_ILL") {
        // A compiler trap instruction raises a real SIGILL, so this also exercises the
        // "faulting instruction address" line.
#    if defined __GNUC__ || defined __clang__
        __builtin_trap();
#    else
        std::raise(SIGILL);
#    endif
    } else if (task == "IGNORE_HUP") {
#    ifdef SIGHUP
        std::raise(SIGHUP);
#    endif
    } else if (const auto sig{ParseSignal(task)}; sig != 0) {
        std::raise(sig);
    }

    // Reached only if the signal handler did not terminate the process.
    std::_Exit(EXIT_SUCCESS);
}

struct SignalTestCase {
    std::string_view task;
    std::vector<std::string> extraArgs;
    int expectedSignal; // 0 means "terminate normally"
    std::string_view expectedText;
};

auto MakeCases() -> std::vector<SignalTestCase> {
    std::vector<SignalTestCase> cases;
    cases.push_back({"INT", {}, SIGINT, "*** INTERRUPT (SIGINT) ***"});
    // The report must also be printed in quiet mode (it does not go through Print).
    cases.push_back({"INT", {"-qq"}, SIGINT, "*** INTERRUPT (SIGINT) ***"});
    cases.push_back({"TERM", {}, SIGTERM, "*** TERMINATE (SIGTERM) ***"});
    cases.push_back({"ABRT", {}, SIGABRT, "*** ABORT (SIGABRT) ***"});
    cases.push_back({"SEGV", {}, SIGSEGV, "*** SEGMENTATION VIOLATION (SIGSEGV) ***"});
    cases.push_back({"FPE", {}, SIGFPE, "*** ERRONEOUS ARITHMETIC OPERATION (SIGFPE) ***"});
    cases.push_back({"ILL", {}, SIGILL, "*** ILLEGAL INSTRUCTION (SIGILL) ***"});
    cases.push_back({"FAULT_NULL", {}, SIGSEGV, "fault address 0x"});
    cases.push_back({"FAULT_ILL", {}, SIGILL, "faulting instruction address 0x"});
#    ifdef SIGHUP
    cases.push_back({"HUP", {}, SIGHUP, "*** HANGUP (SIGHUP) ***"});
    cases.push_back({"IGNORE_HUP", {}, 0, ""});
#    endif
#    ifdef SIGQUIT
    cases.push_back({"QUIT", {}, SIGQUIT, "*** QUIT (SIGQUIT) ***"});
#    endif
#    ifdef SIGBUS
    cases.push_back({"BUS", {}, SIGBUS, "*** BUS ERROR (SIGBUS) ***"});
#    endif
#    ifdef SIGXCPU
    cases.push_back({"XCPU", {}, SIGXCPU, "*** CPU TIME LIMIT EXCEEDED (SIGXCPU) ***"});
#    endif
#    ifdef SIGXFSZ
    cases.push_back({"XFSZ", {}, SIGXFSZ, "*** FILE SIZE LIMIT EXCEEDED (SIGXFSZ) ***"});
#    endif
    return cases;
}

auto ExecutablePath() -> std::string {
    // Main() receives argv + 1, so argv[0] is the subprogram name, not the executable.
#    ifdef __linux__
    std::error_code ec;
    const auto path{std::filesystem::read_symlink("/proc/self/exe", ec)};
    if (not ec) {
        return path.string();
    }
#    elif defined __APPLE__
    std::array<char, PATH_MAX> buffer{};
    auto size{static_cast<std::uint32_t>(buffer.size())};
    if (::_NSGetExecutablePath(buffer.data(), &size) == 0) {
        return buffer.data();
    }
#    endif
    return {};
}

auto ReadAll(int fd, std::string& output) -> void {
    std::array<char, 4096> buffer{};
    while (true) {
        const auto n{::read(fd, buffer.data(), buffer.size())};
        if (n < 0 and errno == EINTR) {
            continue;
        }
        if (n <= 0) {
            break;
        }
        // Keep draining past the cap, so that a large report cannot block the child.
        if (output.size() < maxChildOutput) {
            const auto room{maxChildOutput - output.size()};
            output.append(buffer.data(), std::min(room, static_cast<std::size_t>(n)));
        }
    }
}

auto RunCase(const std::string& executable, const SignalTestCase& testCase) -> bool {
    const auto task{std::string{testCase.task}};
    if (::setenv(childTaskEnvVar, task.c_str(), 1) != 0) {
        fmt::print(stderr, "TestSignalHandler: Failed to set {}.\n", childTaskEnvVar);
        return false;
    }
    if (testCase.task == "FAULT_NULL") {
        ::setenv("MUSTARD_TEST_FAULT_ADDRESS", "0", 1);
    }

    int pipeFD[2]{};
    if (::pipe(pipeFD) != 0) {
        fmt::print(stderr, "TestSignalHandler: Failed to create a pipe.\n");
        return false;
    }

    const auto pid{::fork()};
    if (pid < 0) {
        fmt::print(stderr, "TestSignalHandler: Failed to fork.\n");
        ::close(pipeFD[0]);
        ::close(pipeFD[1]);
        return false;
    }
    if (pid == 0) {
        ::close(pipeFD[0]);
        ::dup2(pipeFD[1], STDERR_FILENO);
        ::close(pipeFD[1]);
        std::vector<char*> childArgv{const_cast<char*>(executable.c_str()),
                                     const_cast<char*>("TestSignalHandler"),
                                     const_cast<char*>("--lite")};
        for (auto&& arg : testCase.extraArgs) {
            childArgv.push_back(const_cast<char*>(arg.c_str()));
        }
        childArgv.push_back(nullptr);
        ::execv(childArgv[0], childArgv.data());
        std::_Exit(127);
    }

    ::close(pipeFD[1]);
    // Drain the capture pipe while waiting: a report larger than the pipe buffer would
    // otherwise block the child, which would then be killed for timing out.
    auto output{std::string{}};
    auto reader{std::thread{[&] {
        ReadAll(pipeFD[0], output);
    }}};
    int status{};
    auto timedOut{false};
    const auto deadline{std::chrono::steady_clock::now() + childTimeout};
    while (true) {
        const auto waited{::waitpid(pid, &status, WNOHANG)};
        if (waited == pid) {
            break;
        }
        if (waited < 0 or std::chrono::steady_clock::now() > deadline) {
            timedOut = true;
            break;
        }
        std::this_thread::sleep_for(10ms);
    }
    if (timedOut) {
        ::kill(pid, SIGKILL);
        ::waitpid(pid, &status, 0);
    }
    reader.join();
    ::close(pipeFD[0]);

    std::string failure;
    if (timedOut) {
        failure = "timed out";
    } else if (testCase.expectedSignal != 0) {
        if (not WIFSIGNALED(status) or WTERMSIG(status) != testCase.expectedSignal) {
            failure = fmt::format("expected termination by signal {}, got status {:#x}",
                                  testCase.expectedSignal, static_cast<unsigned>(status));
        }
    } else if (not WIFEXITED(status) or WEXITSTATUS(status) != EXIT_SUCCESS) {
        failure = fmt::format("expected a normal exit, got status {:#x}", static_cast<unsigned>(status));
    }
    if (failure.empty()) {
        if (testCase.expectedText.empty()) {
            if (output.find("***") != std::string::npos) {
                failure = "expected no report";
            }
        } else if (output.find(testCase.expectedText) == std::string::npos) {
            failure = fmt::format("expected \"{}\" in the report", testCase.expectedText);
        }
    }
    if (not failure.empty()) {
        fmt::print(stderr, "TestSignalHandler: {} failed: {}\n--- child stderr ---\n{}\n---------------------\n",
                   task, failure, output);
        return false;
    }
    return true;
}

auto RunParent() -> int {
    const auto executable{ExecutablePath()};
    if (executable.empty()) {
        fmt::print(stderr, "TestSignalHandler: Failed to locate the test executable.\n");
        return EXIT_FAILURE;
    }
    // Do not leave core files behind from the deliberate faults.
    rlimit noCore{};
    noCore.rlim_cur = 0;
    noCore.rlim_max = 0;
    ::setrlimit(RLIMIT_CORE, &noCore);

    auto nFailure{0};
    for (auto&& testCase : MakeCases()) {
        if (not RunCase(executable, testCase)) {
            ++nFailure;
        }
    }
    if (nFailure != 0) {
        fmt::print(stderr, "TestSignalHandler: {} case(s) failed.\n", nFailure);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

#endif

} // namespace

TestSignalHandler::TestSignalHandler() :
    Subprogram{"TestSignalHandler", "Test Mustard's signal handlers."} {}

auto TestSignalHandler::Main(int argc, char* argv[]) const -> int {
#if defined __unix__ || defined __APPLE__
    if (const auto task{std::getenv(childTaskEnvVar)}) {
        return RunChild(argc, argv, task);
    }
    return RunParent();
#else
    static_cast<void>(argc);
    static_cast<void>(argv);
    fmt::print(stderr, "TestSignalHandler: Skipped, this platform has no POSIX signal handlers.\n");
    return EXIT_SUCCESS;
#endif
}

} // namespace Mustard::Testing
