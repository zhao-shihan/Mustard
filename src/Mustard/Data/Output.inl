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

namespace Mustard::Data {

template<TupleModelizable... Ts>
Output<Ts...>::Output(const std::string& name, const std::string& title,
                      bool enableTimedAutoSave, Second timedAutoSavePeriod) :
    NonCopyableBase{},
    fEntry{},
    fTree{},
    fTimedAutoSaveEnabled{enableTimedAutoSave},
    fTimedAutoSavePeriod{timedAutoSavePeriod},
    fTimedAutoSaveStopwatch{},
    fBranchHelper{fEntry} {
    if (const auto iSlash{name.find_last_of('/')};
        iSlash == std::string::npos) {
        fTree.emplace(name.c_str(), title.c_str());
    } else {
        const auto iName{iSlash + 1};
        const auto dirName{name.substr(0, iName)};
        const auto treeName{name.substr(iName, -1)};
        TDirectory* pwd{gDirectory};
        pwd->mkdir(dirName.c_str());
        pwd->cd(dirName.c_str());
        fTree.emplace(treeName.c_str(), title.c_str());
        fTree->GetDirectory()->Remove(&*fTree);
        gDirectory = pwd;
    }
    [this]<gsl::index... Is>(gslx::index_sequence<Is...>) {
        (...,
         [this]<gsl::index I>(std::integral_constant<gsl::index, I>) {
             using TheValue = std::tuple_element_t<I, Tuple<Ts...>>;
             const auto branch{fBranchHelper.template CreateBranch<TheValue::Name()>(*fTree)};
             branch->SetAutoDelete(false);
             if constexpr (TheValue::Description().has_value()) {
                 const auto title{fmt::format("({}) {}", branch->GetLeaf(TheValue::Name())->GetTypeName(), TheValue::Description().sv())};
                 branch->SetTitle(title.c_str());
             }
         }(std::integral_constant<gsl::index, Is>{}));
    }(gslx::make_index_sequence<Tuple<Ts...>::Size()>());
}

template<TupleModelizable... Ts>
template<typename T>
    requires std::assignable_from<Tuple<Ts...>&, T&&> or ProperSubTuple<Tuple<Ts...>, std::decay_t<T>>
auto Output<Ts...>::Fill(T&& tuple) -> std::size_t {
    const auto nByte{FillImpl<T>(std::forward<T>(tuple))};
    TimedAutoSaveIfNecessary();
    return nByte;
}

template<TupleModelizable... Ts>
template<std::ranges::input_range R>
    requires std::assignable_from<Tuple<Ts...>&, std::ranges::range_reference_t<R>> or
             ProperSubTuple<Tuple<Ts...>, std::ranges::range_value_t<R>>
auto Output<Ts...>::Fill(R&& data) -> std::size_t {
    std::size_t nByte{};
    for (auto&& tuple : std::forward<R>(data)) {
        nByte += FillImpl(muc::forward_like<R>(tuple));
    }
    TimedAutoSaveIfNecessary();
    return nByte;
}

template<TupleModelizable... Ts>
template<std::ranges::input_range R>
    requires std::indirectly_readable<std::ranges::range_reference_t<R>> and
             (std::assignable_from<Tuple<Ts...>&, std::iter_reference_t<std::ranges::range_value_t<R>>> or
              ProperSubTuple<Tuple<Ts...>, std::iter_value_t<std::ranges::range_value_t<R>>>)
auto Output<Ts...>::Fill(R&& data) -> std::size_t {
    std::size_t nByte{};
    for (auto&& i : std::forward<R>(data)) {
        nByte += FillImpl(std::forward<decltype(*i)>(*i));
    }
    TimedAutoSaveIfNecessary();
    return nByte;
}

template<TupleModelizable... Ts>
auto Output<Ts...>::Write(int option, int bufferSize) const -> std::size_t {
    TDirectory* pwd{gDirectory};
    gDirectory = fTree->GetDirectory();
    const auto nByte{fTree->Write(nullptr, option, bufferSize)};
    gDirectory = pwd;
    return nByte;
}

template<TupleModelizable... Ts>
template<typename T>
    requires std::assignable_from<Tuple<Ts...>&, T&&>
auto Output<Ts...>::FillImpl(T&& tuple) -> std::size_t {
    fEntry = std::forward<T>(tuple);
    return fTree->Fill();
}

template<TupleModelizable... Ts>
template<typename T>
    requires ProperSubTuple<Tuple<Ts...>, std::decay_t<T>>
auto Output<Ts...>::FillImpl(T&& tuple) -> std::size_t {
    fEntry = std::move(std::forward<T>(tuple).template As<Tuple<Ts...>>());
    return fTree->Fill();
}

template<TupleModelizable... Ts>
auto Output<Ts...>::TimedAutoSaveIfNecessary() -> std::size_t {
    if (not fTimedAutoSaveEnabled) { return 0; }
    if (Second{fTimedAutoSaveStopwatch.s_elapsed()} < fTimedAutoSavePeriod) { return 0; }
    fTimedAutoSaveStopwatch.reset();
    return fTree->AutoSave("SaveSelf");
}

template<TupleModelizable... Ts>
Output<Ts...>::OutputIterator::OutputIterator(Output* output) :
    fOutput{output} {}

} // namespace Mustard::Data
