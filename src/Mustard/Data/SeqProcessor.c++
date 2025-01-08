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

#include "Mustard/Data/SeqProcessor.h++"

#include "indicators/cursor_control.hpp"

namespace Mustard::Data {

SeqProcessor::SeqProcessor() :
    ProcessorBase{},
    fPrintProgress{true},
    fProgressBar{},
    fWallTimeStopWatch{},
    fLastReportTime{},
    fAsyncReportProgress{} {}

auto SeqProcessor::LoopBeginAction(Index nTotal) -> void {
    if (fPrintProgress) {
        indicators::show_console_cursor(false);
        fProgressBar.emplace(
            indicators::option::BarWidth{16},
            indicators::option::MaxPostfixTextLen{20},
            indicators::option::Start{"["},
            indicators::option::End{"]"},
            indicators::option::ShowElapsedTime{true},
            indicators::option::ShowRemainingTime{true},
            indicators::option::FontStyles{std::vector<indicators::FontStyle>{indicators::FontStyle::bold}},
            indicators::option::MaxProgress{nTotal});
        fWallTimeStopWatch.emplace();
        fLastReportTime = 0;
    }
}

auto SeqProcessor::IterationEndAction(Index nProcessed, Index nTotal) -> void {
    if (fPrintProgress) {
        const auto sElapsed{fWallTimeStopWatch->s_elapsed()};
        if (sElapsed - fLastReportTime > 0.0625) { // report every 62.5ms
            fLastReportTime = sElapsed;
            if (fAsyncReportProgress.valid()) { fAsyncReportProgress.wait(); }
            fAsyncReportProgress = std::async(std::launch::async,
                                              std::mem_fn(&SeqProcessor::ReportProgress),
                                              this, nProcessed, nTotal, sElapsed);
        }
    }
}

auto SeqProcessor::LoopEndAction(Index nTotal) -> void {
    if (fPrintProgress) {
        if (fAsyncReportProgress.valid()) {
            fAsyncReportProgress.wait();
            fProgressBar->set_progress(nTotal);
            fProgressBar->mark_as_completed();
        }
        fProgressBar = std::nullopt;
        indicators::show_console_cursor(true);
        fWallTimeStopWatch = std::nullopt;
        fLastReportTime = 0;
    }
}

auto SeqProcessor::ReportProgress(Index nProcessed, Index nTotal, double sElapsed) -> void {
    fProgressBar->set_option(indicators::option::PostfixText{
        fmt::format("{}/{} ({:.3}/s)", nProcessed, nTotal, nProcessed / sElapsed)});
    fProgressBar->set_progress(nProcessed);
}

} // namespace Mustard::Data
