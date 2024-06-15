#pragma once

#include "Mustard/Env/Memory/WeakSingletonified.h++"
#include "Mustard/Env/Memory/internal/WeakSingletonBase.h++"
#include "Mustard/Env/Memory/internal/WeakSingletonPool.h++"
#include "Mustard/Utility/InlineMacro.h++"

#include "fmt/format.h"

#include <cassert>
#include <memory>
#include <stdexcept>
#include <string>
#include <typeinfo>

namespace Mustard::Env::Memory {

template<typename ADerived>
class PassiveSingleton;

template<typename ADerived>
class WeakSingleton : public internal::WeakSingletonBase {
    friend class PassiveSingleton<ADerived>;

protected:
    WeakSingleton();
    ~WeakSingleton();

public:
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

} // namespace Mustard::Env::Memory

#include "Mustard/Env/Memory/WeakSingleton.inl"
