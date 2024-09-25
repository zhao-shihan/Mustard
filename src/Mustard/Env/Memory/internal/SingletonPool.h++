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

#include "Mustard/Env/Memory/PassiveSingleton.h++"
#include "Mustard/Env/Memory/Singletonified.h++"
#include "Mustard/Utility/NonMoveableBase.h++"
#include "Mustard/Utility/PrettyLog.h++"

#include "gsl/gsl"

#include "fmt/format.h"

#include <memory>
#include <stdexcept>
#include <tuple>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <utility>
#include <vector>

namespace Mustard::Env::Memory::internal {

class SingletonBase;

/// @brief Implementation detail of Mustard::Env::Memory::Singleton.
/// Not API.
class SingletonPool final : public PassiveSingleton<SingletonPool> {
public:
    ~SingletonPool();

    template<Singletonified ASingleton>
    [[nodiscard]] auto Find() -> std::shared_ptr<void*>;
    template<Singletonified ASingleton>
    [[nodiscard]] auto Contains() const -> auto { return fInstanceMap.contains(typeid(ASingleton)); }
    template<Singletonified ASingleton>
    [[nodiscard]] auto Insert(gsl::not_null<ASingleton*> instance) -> std::shared_ptr<void*>;
    [[nodiscard]] auto GetUndeletedInReverseInsertionOrder() const -> std::vector<gsl::owner<const SingletonBase*>>;

private:
    std::unordered_map<std::type_index, const std::tuple<std::weak_ptr<void*>, gsl::index, gsl::owner<const SingletonBase*>>> fInstanceMap;
};

} // namespace Mustard::Env::Memory::internal

#include "Mustard/Env/Memory/internal/SingletonPool.inl"
