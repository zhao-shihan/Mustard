#pragma once

#include "Mustard/Env/BasicEnv.h++"

#include "fmt/core.h"
#include "fmt/ostream.h"

#include <cstdio>
#include <ostream>

namespace Mustard::Env {

/// @brief Verbose level controlled print function.
/// @tparam L Verbose level threshold. Available values are:
///         'E' (Error), 'W' (Warning), 'I' (Informative), 'V' (Verbose).
///         Default to 'E' (always print except when verbose level is `Quiet`).
///         Check `Env::VerboseLevel` and `Env::VerboseLevelReach` for details.
/// @param fmt Same as `fmt::print`
/// @param ...args Same as `fmt::print`
template<char L = 'E', typename... Ts>
auto Print(fmt::format_string<Ts...> fmt, Ts&&... args) -> void;
/// @brief Verbose level controlled print function.
/// @tparam L Verbose level threshold. Available values are:
///         'E' (Error), 'W' (Warning), 'I' (Informative), 'V' (Verbose).
///         Default to 'E' (always print except when verbose level is `Quiet`).
///         Check `Env::VerboseLevel` and `Env::VerboseLevelReach` for details.
/// @param fmt Same as `fmt::println`
/// @param ...args Same as `fmt::println`
template<char L = 'E', typename... Ts>
auto PrintLn(fmt::format_string<Ts...> fmt, Ts&&... args) -> void;
/// @brief Verbose level controlled print function.
/// @tparam L Verbose level threshold. Available values are:
///         'E' (Error), 'W' (Warning), 'I' (Informative), 'V' (Verbose).
///         Default to 'E' (always print except when verbose level is `Quiet`).
///         Check `Env::VerboseLevel` and `Env::VerboseLevelReach` for details.
/// @param fmt Same as `fmt::print`
/// @param ...args Same as `fmt::print`
template<char L = 'E', typename... Ts>
auto Print(std::FILE* f, fmt::format_string<Ts...> fmt, Ts&&... args) -> void;
/// @brief Verbose level controlled print function.
/// @tparam L Verbose level threshold. Available values are:
///         'E' (Error), 'W' (Warning), 'I' (Informative), 'V' (Verbose).
///         Default to 'E' (always print except when verbose level is `Quiet`).
///         Check `Env::VerboseLevel` and `Env::VerboseLevelReach` for details.
/// @param fmt Same as `fmt::println`
/// @param ...args Same as `fmt::println`
template<char L = 'E', typename... Ts>
auto PrintLn(std::FILE* f, fmt::format_string<Ts...> fmt, Ts&&... args) -> void;
/// @brief Verbose level controlled print function.
/// @tparam L Verbose level threshold. Available values are:
///         'E' (Error), 'W' (Warning), 'I' (Informative), 'V' (Verbose).
///         Default to 'E' (always print except when verbose level is `Quiet`).
///         Check `Env::VerboseLevel` and `Env::VerboseLevelReach` for details.
/// @param fmt Same as `fmt::print`
/// @param ...args Same as `fmt::print`
template<char L = 'E', typename... Ts>
auto Print(std::ostream& os, fmt::format_string<Ts...> fmt, Ts&&... args) -> void;
/// @brief Verbose level controlled print function.
/// @tparam L Verbose level threshold. Available values are:
///         'E' (Error), 'W' (Warning), 'I' (Informative), 'V' (Verbose).
///         Default to 'E' (always print except when verbose level is `Quiet`).
///         Check `Env::VerboseLevel` and `Env::VerboseLevelReach` for details.
/// @param fmt Same as `fmt::println`
/// @param ...args Same as `fmt::println`
template<char L = 'E', typename... Ts>
auto PrintLn(std::ostream& os, fmt::format_string<Ts...> fmt, Ts&&... args) -> void;
/// @brief Verbose level controlled print function.
/// @tparam L Verbose level threshold. Available values are:
///         'E' (Error), 'W' (Warning), 'I' (Informative), 'V' (Verbose).
///         Default to 'E' (always print except when verbose level is `Quiet`).
///         Check `Env::VerboseLevel` and `Env::VerboseLevelReach` for details.
/// @param fmt Same as `fmt::print`
/// @param ...args Same as `fmt::print`
template<char L = 'E', typename... Ts>
auto Print(std::wostream& os, fmt::basic_format_string<wchar_t, fmt::type_identity_t<Ts>...> fmt, Ts&&... args) -> void;
/// @brief Verbose level controlled print function.
/// @tparam L Verbose level threshold. Available values are:
///         'E' (Error), 'W' (Warning), 'I' (Informative), 'V' (Verbose).
///         Default to 'E' (always print except when verbose level is `Quiet`).
///         Check `Env::VerboseLevel` and `Env::VerboseLevelReach` for details.
/// @param fmt Same as `fmt::println`
/// @param ...args Same as `fmt::println`
template<char L = 'E', typename... Ts>
auto PrintLn(std::wostream& os, fmt::basic_format_string<wchar_t, fmt::type_identity_t<Ts>...> fmt, Ts&&... args) -> void;
/// @brief Verbose level controlled print function.
/// @tparam L Verbose level threshold. Available values are:
///         'E' (Error), 'W' (Warning), 'I' (Informative), 'V' (Verbose).
///         Default to 'E' (always print except when verbose level is `Quiet`).
///         Check `Env::VerboseLevel` and `Env::VerboseLevelReach` for details.
/// @param ...args Same as `fmt::vprint`
template<char L = 'E'>
auto VPrint(auto&&... args) -> void;

/// @brief Same as `Print<'W'>(stderr, ...)`
/// @param fmt Same as `fmt::print`
/// @param ...args Same as `fmt::print`
template<typename... Ts>
auto PrintWarning(fmt::format_string<Ts...> fmt, Ts&&... args) -> void;
/// @brief Same as `PrintLn<'W'>(stderr, ...)`
/// @param fmt Same as `fmt::println`
/// @param ...args Same as `fmt::println`
template<typename... Ts>
auto PrintLnWarning(fmt::format_string<Ts...> fmt, Ts&&... args) -> void;
/// @brief Same as `VPrint<'W'>(stderr, ...)`
/// @param ...args Same as `fmt::vprint`
template<typename... Ts>
auto VPrintWarning(auto&&... args) -> void;
/// @brief Same as `Print<'E'>(stderr, ...)`
/// @param fmt Same as `fmt::print`
/// @param ...args Same as `fmt::print`
template<typename... Ts>
auto PrintError(fmt::format_string<Ts...> fmt, Ts&&... args) -> void;
/// @brief Same as `PrintLn<'E'>(stderr, ...)`
/// @param fmt Same as `fmt::println`
/// @param ...args Same as `fmt::println`
template<typename... Ts>
auto PrintLnError(fmt::format_string<Ts...> fmt, Ts&&... args) -> void;
/// @brief Same as `VPrint<'E'>(stderr, ...)`
/// @param ...args Same as `fmt::vprint`
template<typename... Ts>
auto VPrintError(auto&&... args) -> void;

} // namespace Mustard::Env

#include "Mustard/Env/Print.inl"
