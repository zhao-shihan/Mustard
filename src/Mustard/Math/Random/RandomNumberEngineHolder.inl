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

namespace Mustard::inline Math::Random {

template<RandomNumberEngineOrReference R>
constexpr RandomNumberEngineHolder<R>::RandomNumberEngineHolder()
    requires(not std::is_reference_v<R>)
= default;

template<RandomNumberEngineOrReference R>
constexpr RandomNumberEngineHolder<R>::RandomNumberEngineHolder(SeedType seed)
    requires(not std::is_reference_v<R>)
    :
    fEngine{seed} {}

template<RandomNumberEngineOrReference R>
constexpr RandomNumberEngineHolder<R>::RandomNumberEngineHolder(EngineType& engine)
    requires(std::is_reference_v<R>)
    :
    fEngine{engine} {}

} // namespace Mustard::inline Math::Random
