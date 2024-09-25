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

#include "Mustard/Env/Memory/Singletonified.h++"
#include "Mustard/Env/Memory/internal/SingletonBase.h++"
#include "Mustard/Env/Memory/internal/SingletonPool.h++"
#include "Mustard/Utility/InlineMacro.h++"
#include "Mustard/Utility/NonConstructibleBase.h++"
#include "Mustard/Utility/PrettyLog.h++"

#include "fmt/format.h"

#include <memory>
#include <stdexcept>
#include <typeinfo>

namespace Mustard::Env::Memory {

template<typename ADerived>
class Singleton : public internal::SingletonBase {
protected:
    Singleton();
    virtual ~Singleton() override;

public:
    MUSTARD_ALWAYS_INLINE static auto Instance() -> ADerived&;
    MUSTARD_ALWAYS_INLINE static auto EnsureInstantiation() -> void { Instance(); }

    MUSTARD_ALWAYS_INLINE static auto NotInstantiated() -> bool { return UpdateInstance() == Status::NotInstantiated; }
    MUSTARD_ALWAYS_INLINE static auto Available() -> bool { return UpdateInstance() == Status::Available; }
    MUSTARD_ALWAYS_INLINE static auto Expired() -> bool { return UpdateInstance() == Status::Expired; }
    MUSTARD_ALWAYS_INLINE static auto Instantiated() -> bool { return not NotInstantiated(); }

private:
    enum struct Status {
        NotInstantiated,
        Available,
        Expired
    };

    MUSTARD_ALWAYS_INLINE static auto UpdateInstance() -> Status;

private:
    static std::shared_ptr<void*> fgInstance;
};

class SingletonInstantiator final : public NonConstructibleBase {
    template<typename ASingleton>
    friend class Singleton;

    template<Singletonified ASingleton>
    static auto New() { return new ASingleton; }
};

} // namespace Mustard::Env::Memory

#include "Mustard/Env/Memory/Singleton.inl"
