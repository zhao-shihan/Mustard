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

namespace Mustard::Env {

template<typename... Ts>
auto PrintPrettyInfo(std::string_view message, const std::source_location& location) -> void {
    PrintInfo(fg(fmt::color::deep_sky_blue), "{}", PrettyInfo(message, location));
    PrintInfo("\n");
}

template<typename... Ts>
auto PrintPrettyWarning(std::string_view message, const std::source_location& location) -> void {
    const auto ts{fmt::emphasis::bold | fg(fmt::color::white) | bg(fmt::color::dark_orange)};
    PrintWarning(ts | fmt::emphasis::blink, "***");
    PrintWarning(ts, " {}", PrettyWarning(message, location));
    PrintWarning("\n");
}

template<typename... Ts>
auto PrintPrettyError(std::string_view message, const std::source_location& location) -> void {
    const auto ts{fmt::emphasis::bold | fg(fmt::color::white) | bg(fmt::color::red)};
    PrintError(ts | fmt::emphasis::blink, "***");
    PrintError(ts, " {}", PrettyError(message, location));
    PrintError("\n");
}

} // namespace Mustard::Env
