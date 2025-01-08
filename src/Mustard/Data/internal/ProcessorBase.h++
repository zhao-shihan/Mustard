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

#include "Mustard/Utility/PrettyLog.h++"

#include "muc/math"

#include "gsl/gsl"

#include "fmt/core.h"

#include <algorithm>
#include <cmath>
#include <concepts>
#include <future>
#include <limits>
#include <stdexcept>
#include <utility>

namespace Mustard::Data::internal {

template<std::integral T>
class ProcessorBase {
public:
    using Index = T;

protected:
    ProcessorBase();
    ~ProcessorBase() = default;

public:
    auto LoadFactor(T val) -> void;
    auto LoadFactor() const -> auto { return fLoadFactor; }

    auto AsyncPolicy(std::launch val) -> void { fAsyncPolicy = val; }
    auto AsyncPolicy() const -> auto { return fAsyncPolicy; }

protected:
    struct BatchConfiguration {
        T nBatch;
        T nEPBQuot;
        T nEPBRem;
    };

    auto CalculateBatchConfiguration(T nProcess, T nTotal) const -> BatchConfiguration;
    static auto CalculateIndexRange(T iBatch, BatchConfiguration batch) -> std::pair<T, T>;

protected:
    double fLoadFactor;
    std::launch fAsyncPolicy;
};

} // namespace Mustard::Data::internal

#include "Mustard/Data/internal/ProcessorBase.inl"
