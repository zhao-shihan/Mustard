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

#include "Mustard/Data/RDFEventSplit.h++"
#include "Mustard/Data/Tuple.h++"
#include "Mustard/Data/TupleModel.h++"
#include "Mustard/Data/internal/ReadHelper.h++"
#include "Mustard/Extension/gslx/index_sequence.h++"
#include "Mustard/Utility/PrettyLog.h++"

#include "ROOT/RDataFrame.hxx"
#include "RtypesCore.h"
#include "TROOT.h"

#include "muc/concepts"
#include "muc/ptrvec"
#include "muc/utility"

#include "gsl/gsl"

#include "fmt/core.h"

#include <atomic>
#include <concepts>
#include <cstddef>
#include <functional>
#include <future>
#include <optional>
#include <semaphore>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace Mustard::Data {

template<typename AData>
class AsyncReader {
public:
    using DataType = AData;

public:
    AsyncReader();
    virtual ~AsyncReader() = 0;

    virtual auto Read(gsl::index first, gsl::index last) -> void;
    virtual auto Acquire() -> AData;

    auto Reading() -> auto { return fReading; }
    auto Exhausted() -> auto { return fExhausted.load(); }

protected:
    std::jthread fReaderThread;
    bool fReading;
    std::binary_semaphore fReadStartSemaphore;
    std::binary_semaphore fReadCompleteSemaphore;
    gsl::index fFirst;
    gsl::index fLast;
    AData fData;
    std::atomic_bool fExhausted;
};

template<TupleModelizable... Ts>
class AsyncEntryReader : public AsyncReader<muc::shared_ptrvec<Tuple<Ts...>>> {
public:
    AsyncEntryReader(ROOT::RDF::RNode data);
};

template<std::size_t N, std::integral AEventIDType, muc::instantiated_from<TupleModel>... Ts>
class AsyncEventReader : public AsyncReader<std::vector<std::tuple<muc::shared_ptrvec<Tuple<Ts>>...>>> {
public:
    AsyncEventReader(std::array<ROOT::RDF::RNode, N> data, std::string eventIDColumnName);
    AsyncEventReader(std::array<ROOT::RDF::RNode, N> data, std::array<std::string, N> eventIDColumnName);
    AsyncEventReader(std::array<ROOT::RDF::RNode, N> data, std::vector<std::array<RDFEntryRange, N>> eventSplit);

private:
    std::vector<std::array<RDFEntryRange, N>> fEventSplit;
};

template<std::integral AEventIDType, TupleModelizable... Ts>
class AsyncEventReader<1, AEventIDType, TupleModel<Ts...>> : public AsyncReader<std::vector<muc::shared_ptrvec<Tuple<Ts...>>>> {
public:
    AsyncEventReader(ROOT::RDF::RNode data, std::string eventIDColumnName);
    AsyncEventReader(ROOT::RDF::RNode data, std::vector<gsl::index> eventSplit);

    virtual auto Read(gsl::index first, gsl::index last) -> void override;

private:
    std::vector<gsl::index> fEventSplit;
};

} // namespace Mustard::Data

#include "Mustard/Data/AsyncReader.inl"
