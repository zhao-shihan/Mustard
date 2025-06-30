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
#include "Mustard/Utility/NonCopyableBase.h++"
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
class AsyncReader : public NonCopyableBase {
public:
    using DataType = AData;

public:
    AsyncReader(gsl::index sentinel, std::function<void(ROOT::RDF::RNode)> ReadLoop, ROOT::RDF::RNode rdf);
    virtual ~AsyncReader() = 0;

    virtual auto Read(gsl::index first, gsl::index last) -> void;
    [[nodiscard]] virtual auto Acquire() -> AData;
    virtual auto Exhaust() -> void;

    auto Reading() const -> auto { return fReading; }
    auto Exhausted() const -> auto { return fExhausted.load(); }

protected:
    auto First() const -> auto { return fFirst; };
    auto Last() const -> auto { return fLast; };
    auto CompleteRead() -> void;

protected:
    AData fData;

private:
    gsl::index fFirst;
    gsl::index fLast;

    gsl::index fSentinel;
    std::jthread fReaderThread;
    std::binary_semaphore fReadStartSemaphore;
    std::binary_semaphore fReadCompleteSemaphore;
    std::atomic_bool fExhausted;
    bool fReading;
};

template<TupleModelizable... Ts>
class AsyncEntryReader : public AsyncReader<muc::shared_ptrvec<Tuple<Ts...>>> {
public:
    AsyncEntryReader(ROOT::RDF::RNode dataFrame);
};

template<std::integral AEventIDType, muc::instantiated_from<TupleModel>... Ts>
class AsyncEventReader : public AsyncReader<std::vector<std::tuple<muc::shared_ptrvec<Tuple<Ts>>...>>> {
public:
    AsyncEventReader(std::array<ROOT::RDF::RNode, sizeof...(Ts)> dataFrame, std::string eventIDColumnName);
    AsyncEventReader(std::array<ROOT::RDF::RNode, sizeof...(Ts)> dataFrame, std::array<std::string, sizeof...(Ts)> eventIDColumnName);
    AsyncEventReader(std::array<ROOT::RDF::RNode, sizeof...(Ts)> dataFrame, std::vector<std::array<RDFEntryRange, sizeof...(Ts)>> eventSplit);

private:
    std::vector<std::array<RDFEntryRange, sizeof...(Ts)>> fEventSplit;
};

template<std::integral AEventIDType, TupleModelizable... Ts>
class AsyncEventReader<AEventIDType, TupleModel<Ts...>> : public AsyncReader<std::vector<muc::shared_ptrvec<Tuple<Ts...>>>> {
public:
    AsyncEventReader(ROOT::RDF::RNode dataFrame, std::string eventIDColumnName);
    AsyncEventReader(ROOT::RDF::RNode dataFrame, std::vector<gsl::index> eventSplit);

    virtual auto Read(gsl::index first, gsl::index last) -> void override;

private:
    std::vector<gsl::index> fEventSplit;
};

} // namespace Mustard::Data

#include "Mustard/Data/AsyncReader.inl"
