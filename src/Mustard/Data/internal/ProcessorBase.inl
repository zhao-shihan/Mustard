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

namespace Mustard::Data::internal {

template<std::integral T>
ProcessorBase<T>::ProcessorBase() :
    fBatchSizeProposal{300000} {}

template<std::integral T>
auto ProcessorBase<T>::CalculateBatchConfiguration(T nProcess, T nTotal) const -> BatchConfiguration {
    const auto nBatchProposal{std::llround(static_cast<double>(nTotal) / fBatchSizeProposal)};
    const auto nBatch{std::clamp(gsl::narrow<T>(nBatchProposal), std::min(nProcess, nTotal), nTotal)};
    const auto nEventPerBatch{muc::div(nTotal, nBatch)};
    return {nBatch, nEventPerBatch.quot, nEventPerBatch.rem};
}

template<std::integral T>
auto ProcessorBase<T>::CalculateIndexRange(T iBatch, BatchConfiguration batch) -> std::pair<T, T> {
    Expects(0 <= iBatch or iBatch < batch.nBatch);
    T iFirst;
    T iLast;
    if (iBatch < batch.nEPBRem) {
        const auto size{batch.nEPBQuot + 1};
        iFirst = iBatch * size;
        iLast = iFirst + size;
    } else {
        iFirst = batch.nEPBRem + iBatch * batch.nEPBQuot;
        iLast = iFirst + batch.nEPBQuot;
    }
    return {iFirst, iLast};
}

} // namespace Mustard::Data::internal
