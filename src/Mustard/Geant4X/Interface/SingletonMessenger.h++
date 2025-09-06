// -*- C++ -*-
//
// Copyright (C) 2020-2025  The Mustard development team
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
#include "Mustard/IO/PrettyLog.h++"
#include "Mustard/Utility/NonCopyableBase.h++"

#include "G4UImessenger.hh"

#include "muc/hash_set"
#include "muc/tuple"
#include "muc/utility"

#include "gsl/gsl"

#include "fmt/core.h"

#include <concepts>
#include <exception>
#include <functional>
#include <tuple>
#include <typeinfo>

namespace Mustard::Geant4X::inline Interface {

/// @brief Geant4 UI messenger with singleton pattern and
/// safe multi-recipient registration
///
/// @tparam ADerived CRTP-derived class type (must inherit from SingletonMessenger)
/// @tparam ARecipients... List of recipient types that can register with this messenger
///
/// @warning Deregistration during delivery will terminate the program
template<typename ADerived, typename... ARecipients>
class SingletonMessenger : public Env::Memory::Singleton<ADerived>,
                           public G4UImessenger {
protected:
    SingletonMessenger();
    ~SingletonMessenger() = default;

public:
    /// @brief RAII wrapper for recipient registration/deregistration
    /// @tparam ARecipient Recipient type to manage
    /// @note Register is noncopyable since we don't want recipienta
    /// being dupilicated or moved!
    template<typename ARecipient>
    friend class Register;
    template<typename ARecipient>
    class Register : public NonCopyableBase {
    public:
        /// @brief Register a recipient with the singleton messenger
        /// @param recipient Recipient's `this`
        Register(gsl::not_null<ARecipient*> recipient);
        /// @brief Automatically deregister recipient on destruction
        ~Register();

    private:
        ARecipient* fRecipient; ///< Pointer to registered recipient
    };

protected:
    /// @brief Deliver action to all registered recipients of specific type
    /// @tparam ARecipient Type of recipients to target
    /// @param Action Callable object accepting ARecipient& parameter
    /// @note Requires ARecipient to be in the template recipient list
    template<typename ARecipient>
        requires muc::tuple_contains_unique_v<std::tuple<ARecipients...>, ARecipient>
    auto Deliver(std::invocable<ARecipient&> auto&& Action) const -> void;
    /// @brief Deliver action to multiple recipient types simultaneously
    /// @tparam Rs Two or more recipient types to target
    /// @param Action Callable object accepting references to all Rs types
    /// @note All Rs types must be in the template recipient list
    template<typename... Rs, typename F>
        requires(sizeof...(Rs) >= 2 and
                 (... and (std::invocable<F &&, Rs&> and muc::tuple_contains_unique_v<std::tuple<ARecipients...>, Rs>)))
    auto Deliver(F&& Action) const -> void;

private:
    mutable bool fDelivering;                                           ///< Delivery state flag (prevents deregistration during delivery)
    std::tuple<muc::flat_hash_set<ARecipients*>...> fRecipientSetTuple; ///< Type-segregated recipient sets
};

} // namespace Mustard::Geant4X::inline Interface

#include "Mustard/Geant4X/Interface/SingletonMessenger.inl"
