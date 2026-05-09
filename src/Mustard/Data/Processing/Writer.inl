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

namespace Mustard::Data::inline Processing {

template<Modelized M>
Writer<M>::Writer(const std::string& name) :
    Writer{name, envparse::parse<envparse::not_set_option::left_blank>("${MUSTARD_DATA_WRITER_DEFAULT_TARGET}")} {
}

template<Modelized M>
Writer<M>::Writer(const std::string& name, std::string_view target) :
    fWriter{} {
    switch (ResolveTarget(target)) {
    case Target::RNTuple:
        fWriter.emplace(std::in_place_type<RNTupleWriter<M>>, name);
        break;
    case Target::TTree:
        fWriter.emplace(std::in_place_type<TTreeWriter<M>>, name);
        break;
    }
}

template<Modelized M>
auto Writer<M>::Fill(const Tuple<M>& tuple) -> void {
    VisitWriter([&](auto& writer) { writer.Fill(tuple); });
}

template<Modelized M>
auto Writer<M>::Fill(Tuple<M>&& tuple) -> void {
    VisitWriter([&](auto& writer) { writer.Fill(std::move(tuple)); });
}

template<Modelized M>
auto Writer<M>::Fill(const ArcTuple<M>& arcTuple) -> void {
    VisitWriter([&](auto& writer) { writer.Fill(arcTuple); });
}

template<Modelized M>
auto Writer<M>::Fill(ArcTuple<M>&& arcTuple) -> void {
    VisitWriter([&](auto& writer) { writer.Fill(std::move(arcTuple)); });
}

template<Modelized M>
template<std::ranges::input_range R>
auto Writer<M>::Fill(R&& data) -> void {
    VisitWriter([&](auto& writer) { writer.Fill(std::forward<R>(data)); });
}

template<Modelized M>
auto Writer<M>::NEntry() const -> std::size_t {
    return VisitWriter([](const auto& writer) -> std::size_t {
        return writer.NEntry();
    });
}

template<Modelized M>
auto Writer<M>::Flush() -> void {
    VisitWriter([](auto& writer) { writer.Flush(); });
}

template<Modelized M>
template<typename F>
auto Writer<M>::VisitWriter(F&& visitor) const -> decltype(auto) {
    return std::visit(std::forward<F>(visitor), *fWriter);
}

template<Modelized M>
template<typename F>
auto Writer<M>::VisitWriter(F&& visitor) -> decltype(auto) {
    return std::visit(std::forward<F>(visitor), *fWriter);
}

template<Modelized M>
auto Writer<M>::ResolveTarget(std::string_view target) -> Target {
    if (target.empty() or target == "RNTuple") {
        return Target::RNTuple;
    }
    if (target == "TTree") {
        return Target::TTree;
    }
    PrintWarning(fmt::format("Unknown data writer target '{}'; defaulting to RNTupleWriter", target));
    return Target::RNTuple;
}

} // namespace Mustard::Data::inline Processing
