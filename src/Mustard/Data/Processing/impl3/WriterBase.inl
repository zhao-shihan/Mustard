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

namespace Mustard::Data::inline Processing::impl3 {

template<Modelized M, typename ADerived>
WriterBase<M, ADerived>::WriterBase() :
    MoveOnlyBase{} {
    if (not gDirectory->IsWritable()) [[unlikely]] {
        PrintWarning("Current ROOT directory is not writable. Please ensure a writable file is opened.");
    }
}

template<Modelized M, typename ADerived>
auto WriterBase<M, ADerived>::Fill(const ArcTuple<M>& arcTuple) -> void {
    if (arcTuple == nullptr) {
        return;
    }
    Fill(*arcTuple);
}

template<Modelized M, typename ADerived>
auto WriterBase<M, ADerived>::Fill(ArcTuple<M>&& arcTuple) -> void {
    if (arcTuple == nullptr) {
        return;
    }
    if (arcTuple->use_count() == 1) {
        Fill(std::move(*arcTuple));
    } else {
        Fill(*arcTuple);
    }
}

template<Modelized M, typename ADerived>
template<std::ranges::input_range R>
auto WriterBase<M, ADerived>::Fill(R&& data) -> void {
    for (auto&& arcTuple : std::forward<R>(data)) {
        Fill(muc::forward_like<R>(arcTuple));
    }
}

template<Modelized M, typename ADerived>
template<typename ATuple>
    requires std::same_as<std::remove_cvref_t<ATuple>, Tuple<M>>
auto WriterBase<M, ADerived>::FillImpl(ATuple&& tuple) -> void {
    const auto assignEntry{[&]<gsl::index I>() {
        using Field = std::tuple_element_t<I, typename M::StdTuple>;
        using PersistentType = typename Field::PersistentType;
        Self().template EntryRef<I>() =
            muc::forward_like<ATuple>(
                std::forward<ATuple>(tuple)
                    .template F<Field::Name(), PersistentType>());
    }};
    [&]<gsl::index... Is>(gslx::index_sequence<Is...>) {
        (..., assignEntry.template operator()<Is>());
    }(gslx::make_index_sequence<M::Size()>{});
    Self().DoFill();
}

template<Modelized M, typename ADerived>
template<typename Field>
auto WriterBase<M, ADerived>::BuildFieldDescription() -> std::string {
    using PersistentType = typename Field::PersistentType;
    if constexpr (Field::Description()) {
        return fmt::format("({}) {}", FieldTypeName<PersistentType>(), Field::Description().sv());
    } else {
        return FieldTypeName<PersistentType>();
    }
}

template<Modelized M, typename ADerived>
template<typename F>
auto WriterBase<M, ADerived>::WithSubdirectory(const std::string& name, F&& create) -> void {
    if (const auto iSlash{name.find_last_of('/')};
        iSlash == std::string::npos) {
        std::invoke(std::forward<F>(create), name);
    } else {
        const auto iName{iSlash + 1};
        const auto dirName{name.substr(0, iName)};
        const auto objName{name.substr(iName, -1)};
        TDirectory* pwd{gDirectory};
        const auto _{gsl::finally([pwd] { gDirectory = pwd; })};
        pwd->mkdir(dirName.c_str());
        if (not pwd->cd(dirName.c_str())) [[unlikely]] {
            PrintError(fmt::format("Failed to change to ROOT directory '{}'.", dirName));
        }
        std::invoke(std::forward<F>(create), objName);
    }
}

} // namespace Mustard::Data::inline Processing::impl3
