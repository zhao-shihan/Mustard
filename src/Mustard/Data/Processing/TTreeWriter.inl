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

namespace Mustard::Data::inline Processing {

template<Modelized M>
TTreeWriter<M>::TTreeWriter(const std::string& name) :
    NonCopyableBase{},
    fEntry{},
    fTree{} {
    if (not gDirectory->IsWritable()) [[unlikely]] {
        PrintWarning("Current ROOT directory is not writable. Please ensure a writable file is opened.");
    }
    // Create tree in the current directory or a subdirectory.
    // The tree will be owned by the writer and not automatically deleted by ROOT.
    if (const auto iSlash{name.find_last_of('/')};
        iSlash == std::string::npos) {
        fTree.emplace(name.c_str(), "");
    } else {
        const auto iName{iSlash + 1};
        const auto dirName{name.substr(0, iName)};
        const auto treeName{name.substr(iName, -1)};
        TDirectory* pwd{gDirectory};
        const auto _{gsl::finally([pwd] { gDirectory = pwd; })};
        pwd->mkdir(dirName.c_str());
        if (not pwd->cd(dirName.c_str())) [[unlikely]] {
            PrintError(fmt::format("Failed to change to ROOT directory '{}'", dirName));
        }
        fTree.emplace(treeName.c_str(), "");
    }
    fTree->GetDirectory()->Remove(&*fTree); // avoid automatic deletion of the tree
    // Create branches
    const auto createBranch{[this]<gsl::index I>() {
        using Value = std::tuple_element_t<I, typename M::StdTuple>;
        using Type = typename Value::Type;
        Type& object{*fEntry.template Get<Value::Name()>()};
        const auto branch{fTree->Branch(Value::Name(), &object)};
        if (branch == nullptr) {
            Throw<std::runtime_error>(fmt::format("Failed to create branch for '{}' field '{}'", FieldTypeName<Value>(), Value::Name().sv()));
        }
        std::string description;
        if constexpr (Value::Description()) {
            description = fmt::format("({}) {}", FieldTypeName<Type>(), Value::Description().sv());
        } else {
            description = FieldTypeName<Type>();
        }
        branch->SetTitle(description.c_str());
    }};
    [&]<gsl::index... Is>(gslx::index_sequence<Is...>) {
        (..., createBranch.template operator()<Is>());
    }(gslx::make_index_sequence<M::Size()>{});
}

template<Modelized M>
TTreeWriter<M>::~TTreeWriter() {
    TDirectory* pwd{gDirectory};
    const auto _{gsl::finally([pwd] { gDirectory = pwd; })};
    gDirectory = fTree->GetDirectory();
    fTree->Write();
}

template<Modelized M>
auto TTreeWriter<M>::Fill(const ArcTuple<M>& arcTuple) -> void {
    if (arcTuple == nullptr) {
        return;
    }
    Fill(*arcTuple);
}

template<Modelized M>
auto TTreeWriter<M>::Fill(ArcTuple<M>&& arcTuple) -> void {
    if (arcTuple == nullptr) {
        return;
    }
    if (arcTuple->use_count() == 1) {
        Fill(std::move(*arcTuple));
    } else {
        Fill(*arcTuple);
    }
}

template<Modelized M>
template<std::ranges::input_range R>
auto TTreeWriter<M>::Fill(R&& data) -> void {
    for (auto&& arcTuple : data) {
        if constexpr (std::is_lvalue_reference_v<R>) {
            Fill(arcTuple);
        } else {
            Fill(std::move(arcTuple));
        }
    }
}

template<Modelized M>
auto TTreeWriter<M>::NEntry() const -> long long {
    const auto nEntry{fTree->GetEntries()};
    return gsl::narrow<long long>(nEntry);
}

template<Modelized M>
template<typename ATuple>
    requires std::same_as<std::remove_cvref_t<ATuple>, Tuple<M>>
auto TTreeWriter<M>::FillImpl(ATuple&& tuple) -> void {
    fEntry = std::forward<ATuple>(tuple);
    fTree->Fill();
}

} // namespace Mustard::Data::inline Processing
