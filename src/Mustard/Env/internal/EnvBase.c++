#include "Mustard/Env/Memory/internal/SingletonDeleter.h++"
#include "Mustard/Env/Memory/internal/SingletonPool.h++"
#include "Mustard/Env/Memory/internal/WeakSingletonPool.h++"
#include "Mustard/Env/internal/EnvBase.h++"

#include "muc/bit"

#include "fmt/format.h"

#include <limits>

#if MUSTARD_SIGNAL_HANDLER

#    include "Mustard/Env/MPIEnv.h++"
#    include "Mustard/Env/Print.h++"
#    include "Mustard/Utility/InlineMacro.h++"
#    include "Mustard/Utility/PrintStackTrace.h++"

#    include "fmt/chrono.h"

#    include <chrono>
#    include <csignal>
#    include <cstdlib>
#    include <ctime>
#    include <exception>
#    include <iostream>

#endif

namespace Mustard::Env::internal {

#if MUSTARD_SIGNAL_HANDLER

namespace {

extern "C" {

auto MUSTARD_SIGINT_SIGTERM_Handler(int sig) -> void {
    std::signal(sig, SIG_DFL);
    if (static auto called{false};
        called) {
        std::abort();
    } else {
        called = true;
    }
    static struct Handler {
        MUSTARD_ALWAYS_INLINE Handler(int sig) {
            const auto now{std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())};
            const auto lineHeader{MPIEnv::Available() ?
                                      fmt::format("MPI{}> ", MPIEnv::Instance().CommWorldRank()) :
                                      ""};
            PrintLn(std::clog, "");
            switch (sig) {
            case SIGINT:
                PrintLn(std::clog, "{}***** INTERRUPT (SIGINT) received", lineHeader);
                break;
            case SIGTERM:
                PrintLn(std::clog, "{}***** TERMINATE (SIGTERM) received", lineHeader);
                break;
            }
            if (MPIEnv::Available()) {
                const auto& mpi{MPIEnv::Instance()};
                PrintLn(std::clog, "{}***** on MPI process {} (node: {})", lineHeader, mpi.CommWorldRank(), mpi.LocalNode().name);
            }
            PrintLn(std::clog, "{}***** at {:%FT%T%z}", lineHeader, fmt::localtime(now));
            PrintStackTrace(64, 2);
            PrintLn(std::clog, "");
            flush(std::clog);
            std::raise(sig);
        }
    } handler{sig};
}

[[noreturn]] auto MUSTARD_SIGABRT_Handler(int) -> void {
    std::signal(SIGABRT, SIG_DFL);
    const auto now{std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())};
    const auto lineHeader{MPIEnv::Available() ?
                              fmt::format("MPI{}> ", MPIEnv::Instance().CommWorldRank()) :
                              ""};
    PrintLn(std::clog, "");
    PrintLn(std::clog, "{}***** ABORT (SIGABRT) received", lineHeader);
    if (MPIEnv::Available()) {
        const auto& mpi{MPIEnv::Instance()};
        PrintLn(std::clog, "{}***** on MPI process {} (node: {})", lineHeader, mpi.CommWorldRank(), mpi.LocalNode().name);
    }
    PrintLn(std::clog, "{}***** at {:%FT%T%z}", lineHeader, fmt::localtime(now));
    PrintStackTrace(64, 2);
    PrintLn(std::clog, "");
    flush(std::clog);
    std::abort();
}

auto MUSTARD_SIGFPE_SIGILL_SIGSEGV_Handler(int sig) -> void {
    std::signal(sig, SIG_DFL);
    if (static auto called{false};
        called) {
        std::abort();
    } else {
        called = true;
    }
    static struct Handler {
        MUSTARD_ALWAYS_INLINE Handler(int sig) {
            const auto now{std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())};
            const auto lineHeader{MPIEnv::Available() ?
                                      fmt::format("MPI{}> ", MPIEnv::Instance().CommWorldRank()) :
                                      ""};
            PrintLn(std::clog, "");
            switch (sig) {
            case SIGFPE:
                PrintLn(std::clog, "{}***** ERRONEOUS ARITHMETIC OPERATION (SIGFPE) received", lineHeader);
                break;
            case SIGILL:
                PrintLn(std::clog, "{}***** ILLEGAL INSTRUCTION (SIGILL) received", lineHeader);
                break;
            case SIGSEGV:
                PrintLn(std::clog, "{}***** SEGMENTATION VIOLATION (SIGSEGV) received", lineHeader);
                break;
            }
            if (MPIEnv::Available()) {
                const auto& mpi{MPIEnv::Instance()};
                PrintLn(std::clog, "{}***** on MPI process {} (node: {})", lineHeader, mpi.CommWorldRank(), mpi.LocalNode().name);
            }
            PrintLn(std::clog, "{}***** at {:%FT%T%z}", lineHeader, fmt::localtime(now));
            PrintStackTrace(64, 2);
            PrintLn(std::clog, "");
            flush(std::clog);
            std::raise(sig);
        }
    } handler{sig};
}

} // extern "C"

std::terminate_handler DefaultTerminateHandler{};

auto TerminateHandler() -> void {
    std::signal(SIGABRT, SIG_DFL);
    if (DefaultTerminateHandler) {
        DefaultTerminateHandler();
    } else {
        Env::PrintLnError("std::terminate called");
        std::abort();
    }
}

} // namespace

#endif

EnvBase::EnvBase() :
    NonMoveableBase{},
    fWeakSingletonPool{},
    fSingletonPool{},
    fSingletonDeleter{} {

    static_assert("三清庇佑 运行稳定 结果无偏");
    static_assert("God bless no bugs");

    CheckFundamentalType();

#if MUSTARD_SIGNAL_HANDLER
    std::signal(SIGABRT, MUSTARD_SIGABRT_Handler);
    std::signal(SIGFPE, MUSTARD_SIGFPE_SIGILL_SIGSEGV_Handler);
    std::signal(SIGILL, MUSTARD_SIGFPE_SIGILL_SIGSEGV_Handler);
    std::signal(SIGINT, MUSTARD_SIGINT_SIGTERM_Handler);
    std::signal(SIGSEGV, MUSTARD_SIGFPE_SIGILL_SIGSEGV_Handler);
    std::signal(SIGTERM, MUSTARD_SIGINT_SIGTERM_Handler);
    DefaultTerminateHandler = std::set_terminate(TerminateHandler);
#endif

    if (static bool gInstantiated = false;
        gInstantiated) {
        throw std::logic_error("Mustard::Env::internal::EnvBase: Trying to construct environment twice");
    } else {
        gInstantiated = true;
    }

    fWeakSingletonPool = std::make_unique_for_overwrite<Memory::internal::WeakSingletonPool>();
    fSingletonPool = std::make_unique_for_overwrite<Memory::internal::SingletonPool>();
    fSingletonDeleter = std::make_unique_for_overwrite<Memory::internal::SingletonDeleter>();
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
            fmt::println(stderr, "Warning: The fundamental data model is LLP64 (not LP64)");
        } else if constexpr (ilp32) {
            fmt::println(stderr, "Warning: The fundamental data model is ILP32 (not LP64)");
        } else if constexpr (lp32) {
            fmt::println(stderr, "Warning: The fundamental data model is LP32 (not LP64)");
        } else {
            fmt::println(stderr, "Warning: Using a rare fundamental data model "
                                 "[{}-bits char, {}-bits short, {}-bits int, {}-bits long, {}-bits long long, {}-bits pointer], "
                                 "run at your own risk",
                         muc::bit_size<char>, muc::bit_size<short>, muc::bit_size<int>, muc::bit_size<long>, muc::bit_size<long long>, muc::bit_size<void*>);
        }
    }
    if (not std::numeric_limits<float>::is_iec559) {
        fmt::println(stderr, "Warning: 'float' does not fulfill the requirements of IEC 559 (IEEE 754)");
    }
    if (not std::numeric_limits<double>::is_iec559) {
        fmt::println(stderr, "Warning: 'double' does not fulfill the requirements of IEC 559 (IEEE 754)");
    }
    if (not std::numeric_limits<long double>::is_iec559) {
        fmt::println(stderr, "Warning: 'long double' does not fulfill the requirements of IEC 559 (IEEE 754)");
    }
}

} // namespace Mustard::Env::internal
