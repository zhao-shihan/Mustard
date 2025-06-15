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

#pragma once

#include "Mustard/Env/BasicEnv.h++"

#include "mpl/mpl.hpp"

#include "fmt/color.h"
#include "fmt/core.h"
#include "fmt/ostream.h"
#include "fmt/xchar.h"

#include <cstdio>
#include <ostream>

namespace Mustard::inline Utility {

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
/// @param ts Same as `fmt::print`
/// @param fmt Same as `fmt::print`
/// @param ...args Same as `fmt::print`
template<char L = 'E', typename... Ts>
auto Print(const fmt::text_style& ts, fmt::format_string<Ts...> fmt, Ts&&... args) -> void;

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
/// @param ts Same as `fmt::print`
/// @param fmt Same as `fmt::print`
/// @param ...args Same as `fmt::print`
template<char L = 'E', typename... Ts>
auto Print(std::FILE* f, const fmt::text_style& ts, fmt::format_string<Ts...> fmt, Ts&&... args) -> void;

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
/// @param ts Same as `fmt::print`
/// @param fmt Same as `fmt::print`
/// @param ...args Same as `fmt::print`
template<char L = 'E', typename... Ts>
auto Print(std::ostream& os, const fmt::text_style& ts, fmt::format_string<Ts...> fmt, Ts&&... args) -> void;

/// @brief Verbose level controlled print function.
/// @tparam L Verbose level threshold. Available values are:
///         'E' (Error), 'W' (Warning), 'I' (Informative), 'V' (Verbose).
///         Default to 'E' (always print except when verbose level is `Quiet`).
///         Check `Env::VerboseLevel` and `Env::VerboseLevelReach` for details.
/// @param fmt Same as `fmt::print`
/// @param ...args Same as `fmt::print`
template<char L = 'E', typename... Ts>
auto Print(std::wostream& os, fmt::wformat_string<Ts...> fmt, Ts&&... args) -> void;
/// @brief Verbose level controlled print function.
/// @tparam L Verbose level threshold. Available values are:
///         'E' (Error), 'W' (Warning), 'I' (Informative), 'V' (Verbose).
///         Default to 'E' (always print except when verbose level is `Quiet`).
///         Check `Env::VerboseLevel` and `Env::VerboseLevelReach` for details.
/// @param fmt Same as `fmt::println`
/// @param ...args Same as `fmt::println`
template<char L = 'E', typename... Ts>
auto PrintLn(std::wostream& os, fmt::wformat_string<Ts...> fmt, Ts&&... args) -> void;
/// @brief Verbose level controlled print function.
/// @tparam L Verbose level threshold. Available values are:
///         'E' (Error), 'W' (Warning), 'I' (Informative), 'V' (Verbose).
///         Default to 'E' (always print except when verbose level is `Quiet`).
///         Check `Env::VerboseLevel` and `Env::VerboseLevelReach` for details.
/// @param ts Same as `fmt::print`
/// @param fmt Same as `fmt::print`
/// @param ...args Same as `fmt::print`
template<char L = 'E', typename... Ts>
auto Print(std::wostream& os, const fmt::text_style& ts, fmt::wformat_string<Ts...> fmt, Ts&&... args) -> void;

/// @brief Verbose level controlled print function.
/// @tparam L Verbose level threshold. Available values are:
///         'E' (Error), 'W' (Warning), 'I' (Informative), 'V' (Verbose).
///         Default to 'E' (always print except when verbose level is `Quiet`).
///         Check `Env::VerboseLevel` and `Env::VerboseLevelReach` for details.
/// @param ...args Same as `fmt::vprint`
template<char L = 'E'>
auto VPrint(auto&&... args) -> void;

/// @brief Verbose level controlled master process print function.
/// @tparam L Verbose level threshold. Available values are:
///         'E' (Error), 'W' (Warning), 'I' (Informative), 'V' (Verbose).
///         Default to 'E' (always print except when verbose level is `Quiet`).
///         Check `Env::VerboseLevel` and `Env::VerboseLevelReach` for details.
/// @param fmt Same as `fmt::print`
/// @param ...args Same as `fmt::print`
template<char L = 'E', typename... Ts>
auto MasterPrint(fmt::format_string<Ts...> fmt, Ts&&... args) -> void;
/// @brief Verbose level controlled master process print function.
/// @tparam L Verbose level threshold. Available values are:
///         'E' (Error), 'W' (Warning), 'I' (Informative), 'V' (Verbose).
///         Default to 'E' (always print except when verbose level is `Quiet`).
///         Check `Env::VerboseLevel` and `Env::VerboseLevelReach` for details.
/// @param fmt Same as `fmt::println`
/// @param ...args Same as `fmt::println`
template<char L = 'E', typename... Ts>
auto MasterPrintLn(fmt::format_string<Ts...> fmt, Ts&&... args) -> void;
/// @brief Verbose level controlled master process print function.
/// @tparam L Verbose level threshold. Available values are:
///         'E' (Error), 'W' (Warning), 'I' (Informative), 'V' (Verbose).
///         Default to 'E' (always print except when verbose level is `Quiet`).
///         Check `Env::VerboseLevel` and `Env::VerboseLevelReach` for details.
/// @param ts Same as `fmt::print`
/// @param fmt Same as `fmt::print`
/// @param ...args Same as `fmt::print`
template<char L = 'E', typename... Ts>
auto MasterPrint(const fmt::text_style& ts, fmt::format_string<Ts...> fmt, Ts&&... args) -> void;

/// @brief Verbose level controlled master process print function.
/// @tparam L Verbose level threshold. Available values are:
///         'E' (Error), 'W' (Warning), 'I' (Informative), 'V' (Verbose).
///         Default to 'E' (always print except when verbose level is `Quiet`).
///         Check `Env::VerboseLevel` and `Env::VerboseLevelReach` for details.
/// @param fmt Same as `fmt::print`
/// @param ...args Same as `fmt::print`
template<char L = 'E', typename... Ts>
auto MasterPrint(std::FILE* f, fmt::format_string<Ts...> fmt, Ts&&... args) -> void;
/// @brief Verbose level controlled master process print function.
/// @tparam L Verbose level threshold. Available values are:
///         'E' (Error), 'W' (Warning), 'I' (Informative), 'V' (Verbose).
///         Default to 'E' (always print except when verbose level is `Quiet`).
///         Check `Env::VerboseLevel` and `Env::VerboseLevelReach` for details.
/// @param fmt Same as `fmt::println`
/// @param ...args Same as `fmt::println`
template<char L = 'E', typename... Ts>
auto MasterPrintLn(std::FILE* f, fmt::format_string<Ts...> fmt, Ts&&... args) -> void;
/// @brief Verbose level controlled master process print function.
/// @tparam L Verbose level threshold. Available values are:
///         'E' (Error), 'W' (Warning), 'I' (Informative), 'V' (Verbose).
///         Default to 'E' (always print except when verbose level is `Quiet`).
///         Check `Env::VerboseLevel` and `Env::VerboseLevelReach` for details.
/// @param ts Same as `fmt::print`
/// @param fmt Same as `fmt::print`
/// @param ...args Same as `fmt::print`
template<char L = 'E', typename... Ts>
auto MasterPrint(std::FILE* f, const fmt::text_style& ts, fmt::format_string<Ts...> fmt, Ts&&... args) -> void;

/// @brief Verbose level controlled master process print function.
/// @tparam L Verbose level threshold. Available values are:
///         'E' (Error), 'W' (Warning), 'I' (Informative), 'V' (Verbose).
///         Default to 'E' (always print except when verbose level is `Quiet`).
///         Check `Env::VerboseLevel` and `Env::VerboseLevelReach` for details.
/// @param fmt Same as `fmt::print`
/// @param ...args Same as `fmt::print`
template<char L = 'E', typename... Ts>
auto MasterPrint(std::ostream& os, fmt::format_string<Ts...> fmt, Ts&&... args) -> void;
/// @brief Verbose level controlled master process print function.
/// @tparam L Verbose level threshold. Available values are:
///         'E' (Error), 'W' (Warning), 'I' (Informative), 'V' (Verbose).
///         Default to 'E' (always print except when verbose level is `Quiet`).
///         Check `Env::VerboseLevel` and `Env::VerboseLevelReach` for details.
/// @param fmt Same as `fmt::println`
/// @param ...args Same as `fmt::println`
template<char L = 'E', typename... Ts>
auto MasterPrintLn(std::ostream& os, fmt::format_string<Ts...> fmt, Ts&&... args) -> void;
/// @brief Verbose level controlled master process print function.
/// @tparam L Verbose level threshold. Available values are:
///         'E' (Error), 'W' (Warning), 'I' (Informative), 'V' (Verbose).
///         Default to 'E' (always print except when verbose level is `Quiet`).
///         Check `Env::VerboseLevel` and `Env::VerboseLevelReach` for details.
/// @param ts Same as `fmt::print`
/// @param fmt Same as `fmt::print`
/// @param ...args Same as `fmt::print`
template<char L = 'E', typename... Ts>
auto MasterPrint(std::ostream& os, const fmt::text_style& ts, fmt::format_string<Ts...> fmt, Ts&&... args) -> void;

/// @brief Verbose level controlled master process print function.
/// @tparam L Verbose level threshold. Available values are:
///         'E' (Error), 'W' (Warning), 'I' (Informative), 'V' (Verbose).
///         Default to 'E' (always print except when verbose level is `Quiet`).
///         Check `Env::VerboseLevel` and `Env::VerboseLevelReach` for details.
/// @param fmt Same as `fmt::print`
/// @param ...args Same as `fmt::print`
template<char L = 'E', typename... Ts>
auto MasterPrint(std::wostream& os, fmt::wformat_string<Ts...> fmt, Ts&&... args) -> void;
/// @brief Verbose level controlled master process print function.
/// @tparam L Verbose level threshold. Available values are:
///         'E' (Error), 'W' (Warning), 'I' (Informative), 'V' (Verbose).
///         Default to 'E' (always print except when verbose level is `Quiet`).
///         Check `Env::VerboseLevel` and `Env::VerboseLevelReach` for details.
/// @param fmt Same as `fmt::println`
/// @param ...args Same as `fmt::println`
template<char L = 'E', typename... Ts>
auto MasterPrintLn(std::wostream& os, fmt::wformat_string<Ts...> fmt, Ts&&... args) -> void;
/// @brief Verbose level controlled master process print function.
/// @tparam L Verbose level threshold. Available values are:
///         'E' (Error), 'W' (Warning), 'I' (Informative), 'V' (Verbose).
///         Default to 'E' (always print except when verbose level is `Quiet`).
///         Check `Env::VerboseLevel` and `Env::VerboseLevelReach` for details.
/// @param ts Same as `fmt::print`
/// @param fmt Same as `fmt::print`
/// @param ...args Same as `fmt::print`
template<char L = 'E', typename... Ts>
auto MasterPrint(std::wostream& os, const fmt::text_style& ts, fmt::wformat_string<Ts...> fmt, Ts&&... args) -> void;

/// @brief Verbose level controlled master process print function.
/// @tparam L Verbose level threshold. Available values are:
///         'E' (Error), 'W' (Warning), 'I' (Informative), 'V' (Verbose).
///         Default to 'E' (always print except when verbose level is `Quiet`).
///         Check `Env::VerboseLevel` and `Env::VerboseLevelReach` for details.
/// @param ...args Same as `fmt::vprint`
template<char L = 'E'>
auto MasterVPrint(auto&&... args) -> void;

} // namespace Mustard::inline Utility

#include "Mustard/Utility/Print.inl"
