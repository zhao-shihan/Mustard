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

#include "Mustard/Env/Memory/PassiveSingleton.h++"
#include "Mustard/Env/Memory/Singletonified.h++"
#include "Mustard/IO/PrettyLog.h++"
#include "Mustard/Utility/NonCopyableBase.h++"

#include "muc/hash_map"
#include "muc/utility"

#include "gsl/gsl"

#include "fmt/format.h"

#include <memory>
#include <mutex>
#include <stdexcept>
#include <tuple>
#include <typeindex>
#include <typeinfo>
#include <utility>
#include <vector>

namespace Mustard::Env::Memory::internal {

class SingletonBase;

/// @brief Implementation detail of Mustard::Env::Memory::Singleton.
/// Not API.
class SingletonPool final : public PassiveSingleton<SingletonPool> {
public:
    SingletonPool();
    ~SingletonPool();

    template<Singletonified ASingleton>
    auto Find() -> std::shared_ptr<void*>;
    template<Singletonified ASingleton>
    auto Contains() const -> auto { return fInstanceMap.contains(typeid(ASingleton)); }
    template<Singletonified ASingleton>
    [[nodiscard]] auto Insert(gsl::not_null<ASingleton*> instance) -> std::shared_ptr<void*>;
    auto GetUndeletedInReverseInsertionOrder() const -> std::vector<gsl::owner<const SingletonBase*>>;

    static auto RecursiveMutex() -> auto& { return fgRecursiveMutex; }

private:
    muc::flat_hash_map<std::type_index, const std::tuple<std::weak_ptr<void*>, gsl::index, gsl::owner<const SingletonBase*>>> fInstanceMap;

    static std::recursive_mutex fgRecursiveMutex;
};

} // namespace Mustard::Env::Memory::internal

#include "Mustard/Env/Memory/internal/SingletonPool.inl"
