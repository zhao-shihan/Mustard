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

namespace Mustard::Data::internal {

template<std::integral T>
ProcessorBase<T>::ProcessorBase(T batchSizeProposal) :
    fBatchSizeProposal{batchSizeProposal} {}

template<std::integral T>
auto ProcessorBase<T>::CalculateIndexRange(T iBatch, T nEPBQuot, T nEPBRem) -> std::pair<T, T> {
    T iFirst;
    T iLast;
    if (iBatch < nEPBRem) {
        const auto size{nEPBQuot + 1};
        iFirst = iBatch * size;
        iLast = iFirst + size;
    } else {
        iFirst = nEPBRem + iBatch * nEPBQuot;
        iLast = iFirst + nEPBQuot;
    }
    return {iFirst, iLast};
}

} // namespace Mustard::Data::internal
