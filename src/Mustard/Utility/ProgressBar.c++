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

#include "muc/chrono"

#include "fmt/core.h"

#include <cmath>
#include <functional>
#include <future>
#include <mutex>

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
        runStopWatch{},
        printStopWatch{},
        asyncPrint{} {}

    indicators::BlockProgressBar progressBar;
    std::size_t progress;
    std::size_t total;
    muc::chrono::stopwatch runStopWatch;
    muc::chrono::stopwatch printStopWatch;
    std::future<void> asyncPrint;

    static std::mutex gPrintMutex;
};

std::mutex ProgressBar::Impl::gPrintMutex{};

ProgressBar::ProgressBar() = default;

ProgressBar::ProgressBar(ProgressBar&& other) noexcept {
    if (other.fImpl) {
        other.fImpl->asyncPrint.wait();
        fImpl = std::move(other.fImpl);
    }
}

ProgressBar::~ProgressBar() {
    if (fImpl) {
        Stop();
    }
}

auto ProgressBar::operator=(ProgressBar&& other) noexcept -> ProgressBar& {
    if (this != &other and other.fImpl) {
        if (fImpl) {
            Stop();
        }
        other.fImpl->asyncPrint.wait();
        fImpl = std::move(other.fImpl);
    }
    return *this;
}

auto ProgressBar::Start(std::size_t nTotal) -> void {
    fImpl = std::make_unique<Impl>(nTotal);
    fImpl->asyncPrint = std::async([this, progress{fImpl->progress}] {
        indicators::show_console_cursor(false);
        fImpl->progressBar.set_option(indicators::option::PostfixText{
            fmt::format("{}/{}", progress, fImpl->total)});
        fImpl->progressBar.set_progress(progress);
    });
    fImpl->runStopWatch.reset();
    fImpl->printStopWatch.reset();
}

auto ProgressBar::Tick(std::chrono::nanoseconds printInterval) -> void {
    ++fImpl->progress;
    if (fImpl->printStopWatch.read() < printInterval) {
        return;
    }
    fImpl->asyncPrint.get();
    fImpl->asyncPrint = std::async(std::mem_fn(&ProgressBar::Print), this, fImpl->progress);
    fImpl->printStopWatch.reset();
}

auto ProgressBar::Complete() -> void {
    fImpl->progress = fImpl->total;
    Stop();
}

auto ProgressBar::Stop() -> void {
    fImpl->asyncPrint.get();
    Print(fImpl->progress);
    fImpl->progressBar.mark_as_completed();
    indicators::show_console_cursor(true);
    fImpl.reset();
}

auto ProgressBar::Print(std::size_t progress) -> void {
    const muc::chrono::seconds<double> secondsElapsed{fImpl->runStopWatch.read()};
    fImpl->progressBar.set_option(indicators::option::PostfixText{
        fmt::format("{}/{} ({:.3}/s)", progress, fImpl->total, progress / secondsElapsed.count())});
    std::lock_guard lock{Impl::gPrintMutex};
    fImpl->progressBar.set_progress(progress);
}

} // namespace Mustard::inline Utility
