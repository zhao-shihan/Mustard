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

namespace Mustard::Geant4X::inline Interface {

template<typename ADerived, typename... ARecipients>
SingletonMessenger<ADerived, ARecipients...>::SingletonMessenger() :
    Env::Memory::Singleton<ADerived>{},
    G4UImessenger{},
    fDelivering{},
    fRecipientSetTuple{} {
    static_assert(std::derived_from<ADerived, SingletonMessenger<ADerived, ARecipients...>>);
    static_assert((... and muc::is_uniquely_contained_in_v<ARecipients, ARecipients...>));
}

template<typename ADerived, typename... ARecipients>
template<typename ARecipient>
SingletonMessenger<ADerived, ARecipients...>::Register<ARecipient>::Register(gsl::not_null<ARecipient*> recipient) :
    NonCopyableBase{},
    fRecipient{recipient} {
    static_assert(muc::is_uniquely_contained_in_v<ARecipient, ARecipients...>);
    get<muc::flat_hash_set<ARecipient*>>(SingletonMessenger::Instance().fRecipientSetTuple).emplace(fRecipient);
}

template<typename ADerived, typename... ARecipients>
template<typename ARecipient>
SingletonMessenger<ADerived, ARecipients...>::Register<ARecipient>::~Register() {
    if (SingletonMessenger::Expired()) {
        return;
    }
    auto& messenger{SingletonMessenger::Instance()};
    if (messenger.fDelivering) {
        PrintError("De-register from SingletonMessenger during delivering (fatal error)");
        std::terminate();
    }
    get<muc::flat_hash_set<ARecipient*>>(messenger.fRecipientSetTuple).erase(fRecipient);
}

template<typename ADerived, typename... ARecipients>
template<typename ARecipient>
    requires muc::is_uniquely_contained_in_v<ARecipient, ARecipients...>
auto SingletonMessenger<ADerived, ARecipients...>::Deliver(std::invocable<ARecipient&> auto&& Action) const -> void {
    const auto& recipientSet{get<muc::flat_hash_set<ARecipient*>>(fRecipientSetTuple)};
    if (recipientSet.empty()) {
        PrintError(fmt::format("Error: {} not registered", muc::try_demangle(typeid(ARecipient).name())));
        return;
    }
    fDelivering = true;
    for (auto&& recipient : recipientSet) {
        std::invoke(std::forward<decltype(Action)>(Action), *recipient);
    }
    fDelivering = false;
}

template<typename ADerived, typename... ARecipients>
template<typename... Rs, typename F>
    requires(sizeof...(Rs) >= 2 and
             (... and (std::invocable<F &&, Rs&> and muc::is_uniquely_contained_in_v<Rs, ARecipients...>)))
auto SingletonMessenger<ADerived, ARecipients...>::Deliver(F&& Action) const -> void {
    (..., Deliver<Rs>(std::forward<F>(Action)));
}

} // namespace Mustard::Geant4X::inline Interface
