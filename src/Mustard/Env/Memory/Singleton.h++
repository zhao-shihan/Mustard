#pragma once

#include "Mustard/Env/Memory/Singletonified.h++"
#include "Mustard/Env/Memory/internal/SingletonBase.h++"
#include "Mustard/Env/Memory/internal/SingletonPool.h++"
#include "Mustard/Utility/InlineMacro.h++"
#include "Mustard/Utility/NonConstructibleBase.h++"

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
