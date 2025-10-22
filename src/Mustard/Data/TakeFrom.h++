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

#pragma once

#include "Mustard/Data/Tuple.h++"
#include "Mustard/Data/internal/ReadHelper.h++"
#include "Mustard/Utility/NonConstructibleBase.h++"
#include "Mustard/gslx/index_sequence.h++"

#include "muc/ptrvec"

#include "gsl/gsl"

#include <memory>
#include <string>
#include <vector>

namespace Mustard::Data {

template<TupleModelizable... Ts>
class Take : public NonConstructibleBase {
public:
    static auto From(ROOT::RDF::RNode rdf) -> muc::shared_ptrvec<Tuple<Ts...>>;

private:
    template<gsl::index... Is>
    class TakeOne;

    template<gsl::index... Is>
    TakeOne(muc::shared_ptrvec<Tuple<Ts...>>, gslx::index_sequence<Is...>) -> TakeOne<Is...>;
};

} // namespace Mustard::Data

#include "Mustard/Data/TakeFrom.inl"
