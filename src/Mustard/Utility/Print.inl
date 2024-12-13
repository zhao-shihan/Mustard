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

namespace Mustard::inline Utility {

template<char L, typename... Ts>
auto Print(fmt::format_string<Ts...> fmt, Ts&&... args) -> void {
    if (not Env::VerboseLevelReach<L>()) { return; }
    fmt::print(std::move(fmt), std::forward<Ts>(args)...);
}

template<char L, typename... Ts>
auto PrintLn(fmt::format_string<Ts...> fmt, Ts&&... args) -> void {
    if (not Env::VerboseLevelReach<L>()) { return; }
    fmt::println(std::move(fmt), std::forward<Ts>(args)...);
}

template<char L, typename... Ts>
auto Print(const fmt::text_style& ts, fmt::format_string<Ts...> fmt, Ts&&... args) -> void {
    if (not Env::VerboseLevelReach<L>()) { return; }
    fmt::print(ts, std::move(fmt), std::forward<Ts>(args)...);
}

template<char L, typename... Ts>
auto Print(std::FILE* f, fmt::format_string<Ts...> fmt, Ts&&... args) -> void {
    if (not Env::VerboseLevelReach<L>()) { return; }
    fmt::print(f, std::move(fmt), std::forward<Ts>(args)...);
}

template<char L, typename... Ts>
auto PrintLn(std::FILE* f, fmt::format_string<Ts...> fmt, Ts&&... args) -> void {
    if (not Env::VerboseLevelReach<L>()) { return; }
    fmt::println(f, std::move(fmt), std::forward<Ts>(args)...);
}

template<char L, typename... Ts>
auto Print(std::FILE* f, const fmt::text_style& ts, fmt::format_string<Ts...> fmt, Ts&&... args) -> void {
    if (not Env::VerboseLevelReach<L>()) { return; }
    fmt::print(f, ts, std::move(fmt), std::forward<Ts>(args)...);
}

template<char L, typename... Ts>
auto Print(std::ostream& os, fmt::format_string<Ts...> fmt, Ts&&... args) -> void {
    if (not Env::VerboseLevelReach<L>()) { return; }
    fmt::print(os, std::move(fmt), std::forward<Ts>(args)...);
}

template<char L, typename... Ts>
auto PrintLn(std::ostream& os, fmt::format_string<Ts...> fmt, Ts&&... args) -> void {
    if (not Env::VerboseLevelReach<L>()) { return; }
    fmt::println(os, std::move(fmt), std::forward<Ts>(args)...);
}

template<char L, typename... Ts>
auto Print(std::ostream& os, const fmt::text_style& ts, fmt::format_string<Ts...> fmt, Ts&&... args) -> void {
    if (not Env::VerboseLevelReach<L>()) { return; }
    fmt::print(os, ts, std::move(fmt), std::forward<Ts>(args)...);
}

template<char L, typename... Ts>
auto Print(std::wostream& os, fmt::basic_format_string<wchar_t, fmt::type_identity_t<Ts>...> fmt, Ts&&... args) -> void {
    if (not Env::VerboseLevelReach<L>()) { return; }
    fmt::print(os, std::move(fmt), std::forward<Ts>(args)...);
}

template<char L, typename... Ts>
auto PrintLn(std::wostream& os, fmt::basic_format_string<wchar_t, fmt::type_identity_t<Ts>...> fmt, Ts&&... args) -> void {
    if (not Env::VerboseLevelReach<L>()) { return; }
    fmt::println(os, std::move(fmt), std::forward<Ts>(args)...);
}

template<char L, typename... Ts>
auto Print(std::wostream& os, const fmt::text_style& ts, fmt::basic_format_string<wchar_t, fmt::type_identity_t<Ts>...> fmt, Ts&&... args) -> void {
    if (not Env::VerboseLevelReach<L>()) { return; }
    fmt::print(os, ts, std::move(fmt), std::forward<Ts>(args)...);
}

template<char L>
auto VPrint(auto&&... args) -> void {
    if (not Env::VerboseLevelReach<L>()) { return; }
    fmt::vprint(std::forward<decltype(args)>(args)...);
}

} // namespace Mustard::inline Utility
