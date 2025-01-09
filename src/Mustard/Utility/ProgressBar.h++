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

#include <chrono>
#include <cstdint>
#include <memory>

namespace Mustard::inline Utility {

/// @brief A progress indicator for sequential program.
/// @note Do not support MPI parallel program.
class ProgressBar {
public:
    ProgressBar();
    ~ProgressBar();

    auto Start(std::size_t nTotal) -> void;
    auto Tick(std::chrono::duration<double> printInterval = std::chrono::milliseconds{33}) -> void;
    auto Complete() -> void;
    auto Stop() -> void;

private:
    auto Print(std::chrono::duration<double> timeElapsed) -> void;

private:
    struct Impl;
    std::unique_ptr<Impl> fImpl;
};

} // namespace Mustard::inline Utility
