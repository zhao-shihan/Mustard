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
RNTupleWriter<M>::RNTupleWriter(const std::string& name) :
    impl3::WriterBase<M, RNTupleWriter<M>>{},
    fEntry{},
    fWriter{} {
    // Create RNTuple model and fields.
    // Exeception will be thrown if the value types are not supported by RNTuple.
    auto model{ROOT::RNTupleModel::Create()};
    const auto createField{[&]<gsl::index I>() {
        using Field = std::tuple_element_t<I, typename M::StdTuple>;
        using PersistentType = typename Field::PersistentType;
        std::get<I>(fEntry) = model->template MakeField<PersistentType>(
            Field::Name(), this->template BuildFieldDescription<Field>());
    }};
    [&]<gsl::index... Is>(gslx::index_sequence<Is...>) {
        (..., createField.template operator()<Is>());
    }(gslx::make_index_sequence<M::Size()>{});
    // Create writer and attach to the current directory or a subdirectory.
    this->WithSubdirectory(name, [&](const std::string& ntupleName) {
        fWriter = ROOT::RNTupleWriter::Append(std::move(model), ntupleName, *gDirectory);
    });
}

template<Modelized M>
auto RNTupleWriter<M>::NEntry() const -> long long {
    const auto nEntry{fWriter->GetNEntries()};
    return gsl::narrow<long long>(nEntry);
}

} // namespace Mustard::Data::inline Processing
