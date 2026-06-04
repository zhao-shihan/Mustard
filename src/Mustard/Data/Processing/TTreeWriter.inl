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
    impl3::WriterBase<M, TTreeWriter<M>>{},
    fEntry{std::make_unique<typename PersistentEntryTuple<typename M::StdTuple>::Type>()},
    fTree{} {
    // Create tree in the current directory or a subdirectory.
    // The tree will be owned by the writer and not automatically deleted by ROOT.
    this->WithSubdirectory(name, [&](const std::string& treeName) {
        fTree = std::make_unique<TTree>(treeName.c_str(), "");
    });
    fTree->GetDirectory()->Remove(fTree.get()); // avoid automatic deletion of the tree
    // Create branches
    const auto createBranch{[this]<gsl::index I>() {
        using Field = std::tuple_element_t<I, typename M::StdTuple>;
        const auto branch{fTree->Branch(Field::Name(), &EntryRef<I>())};
        if (branch == nullptr) {
            Throw<std::runtime_error>(fmt::format("Failed to create branch for '{}' field '{}'.", FieldPersistentTypeName<Field>(), Field::Name().sv()));
        }
        branch->SetTitle(this->template BuildFieldDescription<Field>().c_str());
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
auto TTreeWriter<M>::NEntry() const -> long long {
    const auto nEntry{fTree->GetEntries()};
    return gsl::narrow<long long>(nEntry);
}

} // namespace Mustard::Data::inline Processing
