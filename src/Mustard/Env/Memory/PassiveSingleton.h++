#pragma once

#include "Mustard/Env/Memory/PassiveSingletonified.h++"
#include "Mustard/Env/Memory/WeakSingleton.h++"
#include "Mustard/Env/Memory/internal/PassiveSingletonBase.h++"
#include "Mustard/Env/Memory/internal/WeakSingletonPool.h++"

#include "muc/utility"

#include "fmt/format.h"

#include <memory>
#include <stdexcept>
#include <string>
#include <typeinfo>

namespace Mustard::Env::Memory {

template<typename ADerived>
class PassiveSingleton : public internal::PassiveSingletonBase,
                         public WeakSingleton<ADerived> {
protected:
    PassiveSingleton();
    ~PassiveSingleton() = default;

public:
    MUSTARD_ALWAYS_INLINE static auto Instance() -> ADerived&;

private:
    using Base = WeakSingleton<ADerived>;
};

} // namespace Mustard::Env::Memory

#include "Mustard/Env/Memory/PassiveSingleton.inl"
