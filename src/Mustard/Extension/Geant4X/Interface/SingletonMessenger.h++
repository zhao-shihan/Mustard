#pragma once

#include "Mustard/Env/BasicEnv.h++"
#include "Mustard/Env/Memory/Singleton.h++"
#include "Mustard/Env/Print.h++"

#include "G4UImessenger.hh"

#include "muc/tuple"

#include "gsl/gsl"

#include <concepts>
#include <exception>
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

private:
    mutable bool fDelivering;
    std::tuple<std::unordered_set<ARecipients*>...> fRecipientSetTuple;
};

} // namespace Mustard::inline Extension::Geant4X::inline Interface

#include "Mustard/Extension/Geant4X/Interface/SingletonMessenger.inl"
