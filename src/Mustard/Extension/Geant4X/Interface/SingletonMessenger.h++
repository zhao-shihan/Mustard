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

#include "Mustard/Env/BasicEnv.h++"
#include "Mustard/Env/Memory/Singleton.h++"
#include "Mustard/Utility/PrettyLog.h++"

#include "G4UImessenger.hh"

#include "muc/tuple"
#include "muc/utility"

#include "gsl/gsl"

#include "fmt/core.h"

#include <concepts>
#include <exception>
#include <functional>
#include <tuple>
#include <typeinfo>
#include <unordered_set>

namespace Mustard::inline Extension::Geant4X::inline Interface {

template<typename ADerived, typename... ARecipients>
class SingletonMessenger : public Env::Memory::Singleton<ADerived>,
                           public G4UImessenger {
protected:
    SingletonMessenger();
    ~SingletonMessenger() = default;

public:
    template<typename ARecipient>
    friend class Register;
    template<typename ARecipient>
    class Register final {
    public:
        Register(gsl::not_null<ARecipient*> recipient);
        ~Register();

    private:
        ARecipient* fRecipient;
    };

protected:
    template<typename ARecipient>
        requires muc::tuple_contains_unique_v<std::tuple<ARecipients...>, ARecipient>
    auto Deliver(std::invocable<ARecipient&> auto&& Action) const -> void;
    template<typename... Rs, typename F>
        requires(sizeof...(Rs) >= 2 and
                 (... and (std::invocable<F &&, Rs&> and muc::tuple_contains_unique_v<std::tuple<ARecipients...>, Rs>)))
    auto Deliver(F&& Action) const -> void;

private:
    mutable bool fDelivering;
    std::tuple<std::unordered_set<ARecipients*>...> fRecipientSetTuple;
};

} // namespace Mustard::inline Extension::Geant4X::inline Interface

#include "Mustard/Extension/Geant4X/Interface/SingletonMessenger.inl"
