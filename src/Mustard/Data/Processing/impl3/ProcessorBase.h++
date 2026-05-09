// -*- C++ -*-
//
// Copyright (C) 2020-2025  Mustard developers
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

#include "gsl/gsl"

#include <algorithm>
#include <cmath>
#include <concepts>
#include <cstdlib>
#include <utility>

namespace Mustard::Data::inline Processing::impl3 {

template<std::integral T>
class ProcessorBase {
public:
    using Index = T;

protected:
    ProcessorBase();
    ~ProcessorBase() = default;

public:
    auto BatchSizeProposal(T val) -> void { fBatchSizeProposal = std::max(1, val); }
    auto BatchSizeProposal() const -> auto { return fBatchSizeProposal; }

protected:
    struct BatchConfiguration {
        T count{};
        T size{};
        T remainder{};
    };

    auto CalculateBatchConfiguration(T nProcess, T nTotal) const -> BatchConfiguration;
    static auto CalculateIndexRange(T iBatch, BatchConfiguration batch) -> std::pair<T, T>;

private:
    T fBatchSizeProposal;
};

} // namespace Mustard::Data::inline Processing::impl3

#include "Mustard/Data/Processing/impl3/ProcessorBase.inl"
