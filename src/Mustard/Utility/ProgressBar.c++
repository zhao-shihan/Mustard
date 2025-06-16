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

#include "Mustard/Utility/ProgressBar.h++"

#include "indicators/block_progress_bar.hpp"
#include "indicators/cursor_control.hpp"

#include "muc/time"

#include "gsl/gsl"

#include "fmt/core.h"

#include <cmath>
#include <functional>
#include <future>

namespace Mustard::inline Utility {

struct ProgressBar::Impl {
    Impl(std::size_t nTotal) :
        progressBar{indicators::option::BarWidth{16},
                    indicators::option::MaxPostfixTextLen{20},
                    indicators::option::Start{"["},
                    indicators::option::End{"]"},
                    indicators::option::ShowElapsedTime{true},
                    indicators::option::ShowRemainingTime{true},
                    indicators::option::MaxProgress{nTotal}},
        progress{},
        total{nTotal},
        wallTimeStopWatch{},
        lastPrintTime{},
        asyncPrint{} {}

    indicators::BlockProgressBar progressBar;
    std::size_t progress;
    std::size_t total;
    muc::wall_time_stopwatch<> wallTimeStopWatch;
    std::chrono::duration<double> lastPrintTime;
    std::future<void> asyncPrint;
};

ProgressBar::ProgressBar() = default;

ProgressBar::~ProgressBar() {
    if (fImpl) { Stop(); }
}

auto ProgressBar::Start(std::size_t nTotal) -> void {
    fImpl = std::make_unique<Impl>(nTotal);
    fImpl->asyncPrint = std::async([this] {
        indicators::show_console_cursor(false);
        fImpl->progressBar.set_option(indicators::option::PostfixText{
            fmt::format("{}/{}", fImpl->progress, fImpl->total)});
        fImpl->progressBar.set_progress(fImpl->progress);
    });
}

auto ProgressBar::Tick(std::chrono::duration<double> printInterval) -> void {
    ++fImpl->progress;
    const std::chrono::duration<double> timeElapsed{fImpl->wallTimeStopWatch.s_elapsed()};
    if (timeElapsed - fImpl->lastPrintTime < printInterval) { return; }
    fImpl->lastPrintTime = timeElapsed;
    fImpl->asyncPrint.get();
    fImpl->asyncPrint = std::async(std::mem_fn(&ProgressBar::Print), this, timeElapsed);
}

auto ProgressBar::Complete() -> void {
    fImpl->progress = fImpl->total;
    Stop();
}

auto ProgressBar::Stop() -> void {
    fImpl->asyncPrint.get();
    Print(std::chrono::duration<double>{fImpl->wallTimeStopWatch.s_elapsed()});
    fImpl->progressBar.mark_as_completed();
    indicators::show_console_cursor(true);
    fImpl = nullptr;
}

auto ProgressBar::Print(std::chrono::duration<double> timeElapsed) -> void {
    fImpl->progressBar.set_option(indicators::option::PostfixText{
        fmt::format("{}/{} ({:.3}/s)", fImpl->progress, fImpl->total, fImpl->progress / timeElapsed.count())});
    fImpl->progressBar.set_progress(fImpl->progress);
}

} // namespace Mustard::inline Utility
