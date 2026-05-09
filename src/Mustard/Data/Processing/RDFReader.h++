// -*- C++ -*-
//
// Copyright (C) 2020-2026  Shihan Zhao and contributors
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

#include "Mustard/Data/Container/ArcTupleHashMap.h++"
#include "Mustard/Data/Container/ArcTupleVector.h++"
#include "Mustard/Data/Model.h++"
#include "Mustard/Data/Object/Tuple.h++"
#include "Mustard/Data/Processing/CountRDFEntry.h++"
#include "Mustard/Data/Processing/RDFEventInfo.h++"
#include "Mustard/IO/PrettyLog.h++"
#include "Mustard/Memory/Allocator.h++"
#include "Mustard/Memory/Arc.h++"
#include "Mustard/Utility/NonCopyableBase.h++"
#include "Mustard/gslx/index_sequence.h++"

#include "ROOT/RDataFrame.hxx"
#include "RtypesCore.h"
#include "TROOT.h"

#include "gtl/phmap.hpp"
#include "gtl/vector.hpp"

#include "gsl/gsl"

#include "fmt/format.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <concepts>
#include <cstddef>
#include <functional>
#include <future>
#include <iterator>
#include <ranges>
#include <semaphore>
#include <stdexcept>
#include <string>
#include <thread>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace Mustard::Data::inline Processing {

template<std::signed_integral T, typename D, std::size_t N>
class RDFReader : public NonCopyableBase {
public:
    using Entry = gsl::index;
    using Index = T;
    using Data = std::vector<D, Allocator<D>>;

protected:
    using DataFrameType = std::conditional_t<N == 1, ROOT::RDF::RNode, std::array<ROOT::RDF::RNode, N>>;

protected:
    explicit RDFReader(Index size, std::optional<DataFrameType> rdf = {});
    virtual ~RDFReader() = default;

public:
    [[nodiscard]] auto AsyncRead(Index first, Index last) -> std::future<Data>;
    [[nodiscard]] auto Read(Index first, Index last) -> Data;
    auto Exhaust() -> void;

    auto Reading() const -> auto { return fReading; }
    auto Exhausted() const -> auto { return fExhausted.test(); }

protected:
    auto First() const -> auto { return fFirst; };
    auto Last() const -> auto { return fLast; };
    auto Size() const -> auto { return fSize; }
    auto CompleteRead() -> void;

    // Called by all derived classes to ensure thread-safety!
    auto DestructorAction() -> void;

private:
    virtual auto ReaderKernel(std::optional<DataFrameType>&& rdf) -> void = 0;

protected:
    Index fNext; ///< Next entry/event index
    Arc<Data> fData;

private:
    Index fFirst;
    Index fLast;
    Index fSize;

    std::jthread fReader;
    std::binary_semaphore fStartReadSemaphore;
    std::binary_semaphore fCompleteReadSemaphore;
    std::atomic_flag fExhausted;
    bool fReading;
};

template<Modelized... Ms>
class RDFEntryReader;

template<Modelized M>
class RDFEntryReader<M> : public RDFReader<gsl::index, ArcTuple<M>, 1> {
private:
    using Base = RDFReader<gsl::index, ArcTuple<M>, 1>;

public:
    using typename Base::Entry;

protected:
    using typename Base::DataFrameType;

public:
    explicit RDFEntryReader(DataFrameType rdf);
    explicit RDFEntryReader(DataFrameType rdf, Entry nRDFEntry);
    ~RDFEntryReader() override { this->DestructorAction(); }

    auto NEntry() const -> auto { return this->Size(); }
    auto NextEntry() const -> Entry;

private:
    auto ReaderKernel(std::optional<DataFrameType>&& rdf) -> void override;
};

template<Modelized... Ms>
class RDFEntryReader : public RDFReader<gsl::index, std::tuple<ArcTuple<Ms>...>, sizeof...(Ms)> {
private:
    using N = std::integral_constant<gsl::index, sizeof...(Ms)>;
    using Base = RDFReader<gsl::index, std::tuple<ArcTuple<Ms>...>, N{}>;

public:
    using typename Base::Entry;

protected:
    using typename Base::DataFrameType;

public:
    explicit RDFEntryReader(DataFrameType rdf);
    explicit RDFEntryReader(DataFrameType rdf, const std::array<Entry, N{}>& nRDFEntry);
    ~RDFEntryReader() override { this->DestructorAction(); }

    auto NEntry() const -> auto { return this->Size(); }
    auto NextEntry() const -> Entry;

    template<gsl::index K>
        requires(K < N{})
    auto SubReader() const -> const auto& { return get<K>(fSubReader).value(); }

private:
    auto ReaderKernel(std::optional<DataFrameType>&& rdf) -> void override;

private:
    std::tuple<std::optional<RDFEntryReader<Ms>>...> fSubReader;
};

template<std::integral T, Modelized... Ms>
class RDFEventReader;

template<std::integral T, Modelized M>
class RDFEventReader<T, M> : public RDFReader<std::make_signed_t<T>, ArcTupleVector<M>, 1> {
private:
    using Base = RDFReader<std::make_signed_t<T>, ArcTupleVector<M>, 1>;

public:
    using typename Base::Entry;
    using typename Base::Index;

protected:
    using typename Base::DataFrameType;

public:
    explicit RDFEventReader(DataFrameType rdf, std::string eventIDColumnName);
    explicit RDFEventReader(DataFrameType rdf, Arc<SingleRDFEventInfo<T>> rdfEventInfo);
    ~RDFEventReader() override { this->DestructorAction(); }

    auto NEvent() const -> auto { return this->Size(); }
    auto NextEvtIdx() const -> Index;
    auto NextEntry() const -> Entry;

    auto RDFEventInfo() const -> auto { return fRDFEventInfo; }

private:
    auto ReaderKernel(std::optional<DataFrameType>&& rdf) -> void override;

private:
    Entry fEntry;
    Arc<SingleRDFEventInfo<T>> fRDFEventInfo;
};

template<std::integral T, Modelized... Ms>
class RDFEventReader : public RDFReader<std::make_signed_t<T>, std::tuple<ArcTupleVector<Ms>...>, sizeof...(Ms)> {
private:
    using N = std::integral_constant<gsl::index, sizeof...(Ms)>;
    using Base = RDFReader<std::make_signed_t<T>, std::tuple<ArcTupleVector<Ms>...>, N{}>;

public:
    using typename Base::Data;
    using typename Base::Index;

protected:
    using typename Base::DataFrameType;

public:
    explicit RDFEventReader(DataFrameType rdf, const std::string& eventIDColumnName);
    explicit RDFEventReader(DataFrameType rdf, std::array<std::string, N{}> eventIDColumnName);
    explicit RDFEventReader(DataFrameType rdf, Arc<MultiRDFEventInfo<T, N{}>> rdfEventInfo);
    ~RDFEventReader() override { this->DestructorAction(); }

    auto NEvent() const -> auto { return this->Size(); }
    auto NextEvtIdx() const -> Index;

    auto RDFEventInfo() const -> auto { return fRDFEventInfo; }

    template<gsl::index K>
        requires(K < N{})
    auto SubReader() const -> const auto& { return get<K>(fSubReader).value(); }

    auto MisalignedEventCountWarningThreshold() const -> auto { return fMisalignedEventCountWarningThreshold; }
    auto MisalignedEventCountWarningThreshold(Index n) -> void;
    auto MisalignedEventCountFatalThreshold() const -> auto { return fMisalignedEventCountFatalThreshold; }
    auto MisalignedEventCountFatalThreshold(Index n) -> void;

private:
    auto ReaderKernel(std::optional<DataFrameType>&& rdf) -> void override;

private:
    Arc<MultiRDFEventInfo<T, N{}>> fRDFEventInfo;
    std::tuple<std::optional<RDFEventReader<T, Ms>>...> fSubReader;
    std::array<bool, N{}> fMisalignedEventCountWarningEmitted;
    Index fMisalignedEventCountWarningThreshold;
    Index fMisalignedEventCountFatalThreshold;
};

} // namespace Mustard::Data::inline Processing

#include "Mustard/Data/Processing/RDFReader.inl"
