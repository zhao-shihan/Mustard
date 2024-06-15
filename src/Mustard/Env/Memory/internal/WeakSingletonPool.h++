#pragma once

#include "Mustard/Env/Memory/WeakSingletonified.h++"
#include "Mustard/Utility/NonMoveableBase.h++"

#include "gsl/gsl"

#include "fmt/format.h"

#include <memory>
#include <stdexcept>
#include <string>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>

namespace Mustard::Env::Memory::internal {

/// @brief Implementation detail of Mustard::Env::Memory::WeakSingleton.
/// Not API.
class WeakSingletonPool final : public NonMoveableBase {
public:
    WeakSingletonPool();
    ~WeakSingletonPool();

    static auto Instance() -> WeakSingletonPool&;

    template<WeakSingletonified AWeakSingleton>
    [[nodiscard]] auto Find() -> std::shared_ptr<void*>;
    template<WeakSingletonified AWeakSingleton>
    [[nodiscard]] auto Contains() const -> auto { return fInstanceMap.contains(typeid(AWeakSingleton)); }
    template<WeakSingletonified AWeakSingleton>
    [[nodiscard]] auto Insert(gsl::not_null<AWeakSingleton*> instance) -> std::shared_ptr<void*>;

private:
    std::unordered_map<std::type_index, const std::weak_ptr<void*>> fInstanceMap;

    static WeakSingletonPool* fgInstance;
};

} // namespace Mustard::Env::Memory::internal

#include "Mustard/Env/Memory/internal/WeakSingletonPool.inl"
