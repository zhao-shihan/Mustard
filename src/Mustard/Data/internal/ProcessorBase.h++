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

#include <concepts>
#include <utility>

namespace Mustard::Data::internal {

template<std::integral T>
class ProcessorBase {
public:
    using Index = T;

protected:
    ProcessorBase(T batchSizeProposal = 5'000'000);
    ~ProcessorBase() = default;

public:
    auto BatchSizeProposal() const -> auto { return fBatchSizeProposal; }

    auto BatchSizeProposal(T val) -> void { fBatchSizeProposal = val; }

protected:
    static auto CalculateIndexRange(T iBatch, T nEPBQuot, T nEPBRem) -> std::pair<T, T>;

protected:
    T fBatchSizeProposal;
};

} // namespace Mustard::Data::internal

#include "Mustard/Data/internal/ProcessorBase.inl"
