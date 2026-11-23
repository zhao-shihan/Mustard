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

#include "Mustard/IO/Print.h++"
#include "Mustard/Utility/ProcessTag.h++"

#include "mp1/mp1.hpp"

#include "muc/utility"

#include "fmt/color.h"

#include <csignal>
#include <exception>
#include <string_view>
#include <typeinfo>

namespace Mustard::Env::impl {

[[noreturn]] auto TerminateHandler() -> void {
    const auto procTag{mp1::available() ? fmt::format("{}> ", ProcessTag()) : ""};
    if (const auto exception{std::current_exception()}) {
        std::signal(SIGABRT, SIG_DFL);
        try {
            std::rethrow_exception(exception);
        } catch (const std::exception& e) {
            std::string_view what{e.what()};
            if (not what.empty() and what.ends_with('\n')) {
                what.remove_suffix(1);
            }
            const auto ts{fmt::emphasis::bold | fg(fmt::color::white) | bg(fmt::color::tomato)};
            Print<'E'>(stderr, ts, "{}", procTag);
            Print<'E'>(stderr, ts | fmt::emphasis::blink, "***");
            Print<'E'>(stderr, ts, " terminate called after throwing an instance of '{}'\n", muc::try_demangle(typeid(e).name()));
            Print<'E'>(stderr, ts, "{}", procTag);
            Print<'E'>(stderr, ts | fmt::emphasis::blink, "***");
            Print<'E'>(stderr, ts, "   what(): {}", what);
            Print<'E'>(stderr, "\n");
        } catch (...) {
            const auto ts{fmt::emphasis::bold | fg(fmt::color::white) | bg(fmt::color::tomato)};
            Print<'E'>(stderr, ts, "{}", procTag);
            Print<'E'>(stderr, ts | fmt::emphasis::blink, "***");
            if constexpr (requires { std::current_exception().__cxa_exception_type()->name(); }) {
                Print<'E'>(stderr, ts, " terminate called after throwing an instance of '{}'", muc::try_demangle(std::current_exception().__cxa_exception_type()->name()));
            } else {
                Print<'E'>(stderr, ts, " terminate called after throwing a non-std::exception instance");
            }
            Print<'E'>(stderr, "\n");
        }
    } else {
        const auto ts{fmt::emphasis::bold | fg(fmt::color::white) | bg(fmt::color::dark_orange)};
        Print<'E'>(stderr, ts, "{}", procTag);
        Print<'E'>(stderr, ts | fmt::emphasis::blink, "***");
        Print<'E'>(stderr, ts, " terminate called without an active exception");
        Print<'E'>(stderr, "\n");
    }
    std::abort();
}

} // namespace Mustard::Env::impl
